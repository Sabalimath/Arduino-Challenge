/*
 * Gesture Hero - Arduino Nano 33 BLE Sense
 *
 * Capture les mouvements IMU, fait l'inférence avec le modèle Edge Impulse,
 * et envoie le geste détecté via Serial USB vers le PC.
 *
 * Gestes reconnus : haut, bas, gauche, droite
 *
 * Connexion : Arduino Nano 33 BLE Sense → USB → PC
 */

#include <Arduino_LSM9DS1.h>

// Remplace par le header généré par Edge Impulse
// #include <gesture_hero_inferencing.h>

#define CONFIDENCE_THRESHOLD  0.7
#define SAMPLE_COUNT          50    // nombre d'échantillons par fenêtre
#define SAMPLE_INTERVAL_MS    20    // intervalle entre chaque échantillon (ms)

// Labels des gestes (doit correspondre à ton projet Edge Impulse)
const char* GESTURES[] = { "haut", "bas", "gauche", "droite", "idle" };

float features[SAMPLE_COUNT * 6]; // ax, ay, az, gx, gy, gz

// ─── Collecte des données IMU ──────────────────────────────────────────────

bool collectSamples() {
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    float ax, ay, az, gx, gy, gz;

    if (!IMU.accelerationAvailable() || !IMU.gyroscopeAvailable()) {
      return false;
    }

    IMU.readAcceleration(ax, ay, az);
    IMU.readGyroscope(gx, gy, gz);

    features[i * 6 + 0] = ax;
    features[i * 6 + 1] = ay;
    features[i * 6 + 2] = az;
    features[i * 6 + 3] = gx;
    features[i * 6 + 4] = gy;
    features[i * 6 + 5] = gz;

    delay(SAMPLE_INTERVAL_MS);
  }
  return true;
}

// ─── Setup ────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("ERROR: IMU non détecté");
    while (1);
  }

  Serial.println("READY");
}

// ─── Loop ─────────────────────────────────────────────────────────────────

void loop() {
  if (!collectSamples()) return;

  /*
   * ── Inférence Edge Impulse ──────────────────────────────────────────────
   * Décommente ce bloc une fois ton modèle exporté depuis Edge Impulse
   * et la bibliothèque installée dans l'IDE Arduino.
   *
   * signal_t signal;
   * numpy::signal_from_buffer(features, SAMPLE_COUNT * 6, &signal);
   *
   * ei_impulse_result_t result;
   * EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
   *
   * if (err != EI_IMPULSE_OK) return;
   *
   * int best = 0;
   * for (int i = 1; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
   *   if (result.classification[i].value > result.classification[best].value)
   *     best = i;
   * }
   *
   * if (result.classification[best].value >= CONFIDENCE_THRESHOLD) {
   *   Serial.println(result.classification[best].label);
   * }
   */

  // ── Mode test sans modèle : envoie un geste aléatoire toutes les secondes
  // À supprimer une fois le modèle intégré
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 1000) {
    int idx = random(0, 4);
    Serial.println(GESTURES[idx]);
    lastSend = millis();
  }
}

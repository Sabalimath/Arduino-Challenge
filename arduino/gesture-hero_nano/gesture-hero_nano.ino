/* Gesture Hero - Arduino Nano 33 BLE Sense
 * Inférence Edge Impulse + envoi du geste via Serial USB
 */

#include <gesture-hero_inferencing.h>
#include <Arduino_LSM9DS1.h>

#define CONVERT_G_TO_MS2    9.80665f
#define MAX_ACCEPTED_RANGE  2.0f
#define CONFIDENCE_THRESHOLD 0.7f

static bool debug_nn = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Erreur IMU !");
    while (true);
  }

  if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 3) {
    Serial.println("ERREUR: le modele doit utiliser 3 axes (accelerometre)");
    while (true);
  }

  Serial.println("Gesture Hero pret !");
}

void loop() {
  float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = {0};

  // Collecte des echantillons IMU
  for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
    uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);

    IMU.readAcceleration(buffer[ix], buffer[ix+1], buffer[ix+2]);

    for (int i = 0; i < 3; i++) {
      if (fabs(buffer[ix+i]) > MAX_ACCEPTED_RANGE)
        buffer[ix+i] = (buffer[ix+i] >= 0 ? 1.0f : -1.0f) * MAX_ACCEPTED_RANGE;
    }
    buffer[ix+0] *= CONVERT_G_TO_MS2;
    buffer[ix+1] *= CONVERT_G_TO_MS2;
    buffer[ix+2] *= CONVERT_G_TO_MS2;

    delayMicroseconds(next_tick - micros());
  }

  // Créer le signal
  signal_t signal;
  int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) return;

  // Inférence
  ei_impulse_result_t result = {0};
  err = run_classifier(&signal, &result, debug_nn);
  if (err != EI_IMPULSE_OK) return;

  // Trouver le meilleur geste
  float best_score = 0;
  const char* best_label = nullptr;
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (result.classification[i].value > best_score) {
      best_score = result.classification[i].value;
      best_label = result.classification[i].label;
    }
  }

  // Envoyer tous les scores au format "left: 0.85, right: 0.12"
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    Serial.print(result.classification[i].label);
    Serial.print(": ");
    Serial.print(result.classification[i].value, 2);
    if (i < EI_CLASSIFIER_LABEL_COUNT - 1) Serial.print(", ");
  }
  Serial.println();
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_ACCELEROMETER
#error "Invalid model for current sensor"
#endif

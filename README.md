# Gesture Hero — Rise of the Motion Masters

> Contrôle un véhicule dans **Beach Buggy Racing** uniquement avec des gestes de la main — sans jamais toucher un clavier.

Projet réalisé lors du **Hackathon Arduino Days 2026 - Cotonou**.

---

## Le jeu : Beach Buggy Racing

<p align="center">
  <img src="assets/bbr_gameplay2.webp" width="45%" alt="Beach Buggy Racing - départ"/>
  <img src="assets/bbr_gameplay1.webp" width="45%" alt="Beach Buggy Racing - course"/>
</p>
<p align="center">
  <img src="assets/bbr_splitscreen.webp" width="60%" alt="Beach Buggy Racing - multijoueur"/>
</p>

---

## Comment ça marche

```
[Main] ──USB Serial──► [PC]
Arduino                  Python script
Nano 33                  (gesture_controller.py)
BLE Sense                  │
  │                        ▼
  IMU (accél + gyro)    pynput simule
  Edge Impulse model    les touches clavier
  → détecte le geste    → Beach Buggy Racing
```

1. **Arduino Nano 33 BLE Sense** : lit l'IMU (accéléromètre + gyroscope), fait tourner le modèle IA (Edge Impulse) embarqué, envoie le geste détecté via Serial USB.
2. **Script Python** : écoute le port série et convertit chaque geste en appui de touche clavier.

---

## Gestes reconnus

| Geste   | Touche simulée | Action dans le jeu |
|---------|----------------|--------------------|
| haut    | ↑              | Accélérer          |
| bas     | ↓              | Freiner / Reculer  |
| gauche  | ←              | Tourner à gauche   |
| droite  | →              | Tourner à droite   |

---

## Matériel nécessaire

- Arduino Nano 33 BLE Sense + câble USB
- PC avec Python 3.x
- Beach Buggy Racing (installé sur le PC)

---

## Installation

### 1. Modèle Edge Impulse → Arduino

1. Entraîne ton modèle sur [Edge Impulse Studio](https://studio.edgeimpulse.com) avec les 4 gestes ci-dessus.
2. Exporte le modèle en **Arduino library** (`.zip`).
3. Dans l'IDE Arduino : *Sketch → Include Library → Add .ZIP Library* → sélectionne le `.zip`.
4. Dans `arduino/gesture_hero.ino`, décommente le bloc `#include <gesture_hero_inferencing.h>` et le bloc d'inférence.
5. Flashe la carte.

### 2. Script Python

```bash
pip install pyserial pynput
python python/gesture_controller.py
```

Le script détecte automatiquement le port de l'Arduino.

---

## Structure du projet

```
gesture-hero/
├── arduino/
│   └── gesture_hero.ino          # Code Arduino (IMU + inférence + Serial)
├── esp32/                         # Ancienne version BLE (non utilisée)
│   └── gesture_relay_esp32/
├── python/
│   └── gesture_controller.py     # Serial → clavier (pynput)
├── model/                         # Mettre ici l'export Edge Impulse (.zip)
└── README.md
```

---

## Entraînement du modèle IA (Edge Impulse)

### Étape 1 — Collecte des données

Configuration : capteur **6 axes** (ax, ay, az, gx, gy, gz) @ **62 Hz**, fenêtre de **1000 ms** par échantillon. Appareil : `gesture-nano`.

<p align="center">
  <img src="assets/model/setup_data_acquisition.png" width="70%" alt="Configuration Edge Impulse"/>
</p>

5 labels collectés : **idle**, **up**, **down**, **left**, **right**.

<p align="center">
  <img src="assets/model/idle_first_sample.png" width="48%" alt="Premier échantillon idle"/>
  <img src="assets/model/up_samples.png" width="48%" alt="Échantillons up"/>
</p>
<p align="center">
  <img src="assets/model/down_samples.png" width="48%" alt="Échantillons down"/>
  <img src="assets/model/left_samples.png" width="48%" alt="Échantillons left"/>
</p>
<p align="center">
  <img src="assets/model/dataset_overview.png" width="70%" alt="Vue d'ensemble du dataset"/>
</p>

---

### Étape 2 — Création de l'impulse

Pipeline : **Time series data** → **Spectral features** → **Classifier**

<p align="center">
  <img src="assets/model/create_impulse.png" width="70%" alt="Création de l'impulse"/>
</p>

---

### Étape 3 — Génération des features

**1 237 fenêtres** générées à partir des données collectées :

| Label  | Fenêtres |
|--------|----------|
| idle   | 408      |
| up     | 358      |
| down   | 176      |
| left   | 120      |
| right  | 85       |
| **Total** | **1 237** |

<p align="center">
  <img src="assets/model/generate_features.png" width="48%" alt="Génération des features"/>
  <img src="assets/model/features_result.png" width="48%" alt="Features générées — 13ms, 2KB RAM"/>
</p>

> Performance on-device (Cortex-M4F 80MHz) : **13 ms** de traitement, **2 KB** de RAM.

---

### Étape 4 — Entraînement du réseau de neurones

Architecture : `Input (78 features)` → `Dense (20)` → `Dense (10)` → `Output (5 classes)`

Paramètres : **30 epochs**, learning rate **0.0005**, CPU.

<p align="center">
  <img src="assets/model/nn_settings.png" width="70%" alt="Paramètres du réseau de neurones"/>
</p>

---

### Étape 5 — Résultats

<p align="center">
  <img src="assets/model/model_results2.png" width="70%" alt="Résultats du modèle"/>
</p>

| Métrique   | Valeur |
|------------|--------|
| Accuracy   | **90.3 %** |
| Loss       | 0.34   |

---

### Étape 6 — Déploiement (Arduino library)

Export en **Arduino library (.zip)** via le compilateur EON. Intégration dans l'IDE Arduino via `Sketch > Include Library > Add .ZIP Library`.

<p align="center">
  <img src="assets/model/deployment.png" width="48%" alt="Options de déploiement"/>
  <img src="assets/model/export_success.png" width="48%" alt="Export Arduino library réussi"/>
</p>

---

## Hackathon

- **Événement** : Arduino Days 2026 — Cotonou
- **Défi** : Gesture Hero — Rise of the Motion Masters
- **Dates** : 28-29 Mars 2026

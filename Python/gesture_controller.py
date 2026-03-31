"""
Gesture Hero - Contrôleur clavier
Lit les gestes depuis l'Arduino (Serial USB) et simule les touches.

Installation :
    pip install pyserial pynput serial

Usage :
    python gesture_controller.py
"""

import serial
import serial.tools.list_ports
import sys
import time
from pynput.keyboard import Key, Controller

# ---- Config ----
BAUD_RATE     = 115200
HOLD_DURATION = 0.15   # secondes d'appui sur la touche

# ---- Mapping geste → touche ----
GESTURE_MAP = {
    "haut":   Key.up,
    "bas":    Key.down,
    "gauche": Key.left,
    "droite": Key.right,
}

keyboard = Controller()

def press_key(key):
    keyboard.press(key)
    time.sleep(HOLD_DURATION)
    keyboard.release(key)

def find_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        desc = (p.description or "").lower()
        if any(k in desc for k in ["arduino", "nano", "mbed", "usbmodem"]):
            return p.device
    if ports:
        return ports[0].device
    return None

def main():
    port = find_port()
    if not port:
        print("Aucun port trouvé. Branche l'Arduino et relance.")
        sys.exit(1)

    print(f"Port détecté : {port}")
    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
    except serial.SerialException as e:
        print(f"Erreur ouverture port : {e}")
        sys.exit(1)

    print("En attente de gestes... (Ctrl+C pour arrêter)\n")
    print(f"{'Geste':<10} → Touche")
    print("-" * 25)

    try:
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode("utf-8", errors="ignore").strip().lower()
                if line in GESTURE_MAP:
                    print(f"  {line:<8} → {GESTURE_MAP[line]}")
                    press_key(GESTURE_MAP[line])
    except KeyboardInterrupt:
        print("\nArrêté.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()

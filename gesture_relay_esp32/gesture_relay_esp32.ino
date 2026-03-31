/*
 * Gesture Hero - ESP32 (pont BLE → Serial USB)
 *
 * Se connecte automatiquement à l'Arduino "BLE_GestureHero",
 * reçoit les gestes et les retransmet au PC via Serial.
 *
 * Bibliothèque : ESP32 BLE Arduino (incluse avec le board ESP32)
 */

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEClient.h>

// ---- Mêmes UUIDs que l'Arduino ----
static BLEUUID serviceUUID("19B10000-E8F2-537E-4F6C-D104768A1214");
static BLEUUID charUUID   ("19B10001-E8F2-537E-4F6C-D104768A1214");

#define TARGET_DEVICE_NAME  "BLE_GestureHero"

static BLEClient*              pClient    = nullptr;
static BLERemoteCharacteristic* pChar     = nullptr;
static bool connected = false;
static bool doScan    = true;
static BLEAddress* pServerAddress = nullptr;

// ---- Callback notification BLE → Serial ----
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteChar,
                            uint8_t* pData, size_t length, bool isNotify) {
  String gesture = "";
  for (size_t i = 0; i < length; i++) gesture += (char)pData[i];
  gesture.trim();
  Serial.println(gesture);  // Envoi immédiat au PC
}

// ---- Callback connexion / déconnexion ----
class MyClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) override {
    connected = true;
    Serial.println("[BLE] Connecté à l'Arduino !");
  }
  void onDisconnect(BLEClient* pclient) override {
    connected = false;
    doScan    = true;
    Serial.println("[BLE] Déconnecté, rescan...");
  }
};

// ---- Callback scan ----
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    if (advertisedDevice.getName() == TARGET_DEVICE_NAME) {
      Serial.print("[BLE] Appareil trouvé : ");
      Serial.println(advertisedDevice.toString().c_str());
      BLEDevice::getScan()->stop();
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doScan = false;
    }
  }
};

// ---- Connexion au serveur ----
bool connectToServer(BLEAddress address) {
  Serial.print("[BLE] Connexion à "); Serial.println(address.toString().c_str());

  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallbacks());
  if (!pClient->connect(address)) {
    Serial.println("[BLE] Échec connexion");
    return false;
  }

  BLERemoteService* pService = pClient->getService(serviceUUID);
  if (!pService) {
    Serial.println("[BLE] Service introuvable");
    pClient->disconnect();
    return false;
  }

  pChar = pService->getCharacteristic(charUUID);
  if (!pChar) {
    Serial.println("[BLE] Caractéristique introuvable");
    pClient->disconnect();
    return false;
  }

  if (pChar->canNotify()) {
    pChar->registerForNotify(notifyCallback);
    Serial.println("[BLE] Notifications activées ✓");
  }

  return true;
}

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  Serial.println("=== Gesture Hero ESP32 Bridge ===");

  BLEDevice::init("ESP32_GestureRelay");
  BLEScan* pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pScan->setActiveScan(true);
  pScan->start(5, false);
}

// ---- Boucle ----
void loop() {
  // Si on a trouvé l'adresse et pas encore connecté
  if (!connected && pServerAddress != nullptr) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("[OK] Prêt à relayer les gestes !");
    } else {
      // Relancer un scan
      delete pServerAddress;
      pServerAddress = nullptr;
      doScan = true;
    }
  }

  // Relancer le scan si nécessaire
  if (doScan && pServerAddress == nullptr) {
    Serial.println("[BLE] Scan en cours...");
    BLEDevice::getScan()->start(5, false);
    doScan = false;
  }

  delay(100);
}

#include "BLEManager.h"

BLEManager::BLEManager() 
  : _pServer(nullptr), 
    _pGpsCharacteristic(nullptr), 
    _deviceConnected(false), 
    _oldDeviceConnected(false), 
    _lastNotifyTime(0) {}

void BLEManager::begin(const char* deviceName) {
  BLEDevice::init(deviceName);

  _pServer = BLEDevice::createServer();
  _pServer->setCallbacks(this);

  BLEService* pService = _pServer->createService(SERVICE_UUID);

  _pGpsCharacteristic = pService->createCharacteristic(
                          GPS_CHAR_UUID,
                          BLECharacteristic::PROPERTY_READ |
                          BLECharacteristic::PROPERTY_NOTIFY
                        );

  _pGpsCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  
  BLEDevice::startAdvertising();
  Serial.printf("[BLE] Server started as '%s'. Advertising...\n", deviceName);
}

void BLEManager::onConnect(BLEServer* pServer) {
  _deviceConnected = true;
  Serial.println("[BLE] Smartphone Client Connected!");
}

void BLEManager::onDisconnect(BLEServer* pServer) {
  _deviceConnected = false;
  Serial.println("[BLE] Smartphone Client Disconnected!");
}

void BLEManager::update(const GPSData& gpsData) {
  if (!_deviceConnected && _oldDeviceConnected) {
    delay(500);
    _pServer->startAdvertising();
    Serial.println("[BLE] Restarting Advertising...");
    _oldDeviceConnected = _deviceConnected;
  }

  if (_deviceConnected && !_oldDeviceConnected) {
    _oldDeviceConnected = _deviceConnected;
  }

  if (_deviceConnected && (millis() - _lastNotifyTime > NOTIFY_INTERVAL)) {
    _lastNotifyTime = millis();

    char payload[64];
    snprintf(payload, sizeof(payload), "%.5f,%.5f,%.1f,%u,%d",
             gpsData.latitude,
             gpsData.longitude,
             gpsData.altitudeMeters,
             gpsData.satellites,
             gpsData.fixValid ? 1 : 0);

    _pGpsCharacteristic->setValue(payload);
    _pGpsCharacteristic->notify();

    Serial.printf("[BLE Notify] Sent: %s\n", payload);
  }
}

bool BLEManager::isConnected() const {
  return _deviceConnected;
}
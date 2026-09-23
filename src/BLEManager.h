#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "GPSManager.h"

#define SERVICE_UUID           "12345678-1234-1234-1234-123456789999"
#define GPS_CHAR_UUID          "87654321-4321-4321-4321-999987654321"

class BLEManager : public BLEServerCallbacks {
public:
  BLEManager();
  void begin(const char* deviceName = "TrailGuard-S3");
  void update(const GPSData& gpsData);
  bool isConnected() const;

  void onConnect(BLEServer* pServer) override;
  void onDisconnect(BLEServer* pServer) override;

private:
  BLEServer* _pServer;
  BLECharacteristic* _pGpsCharacteristic;
  bool _deviceConnected;
  bool _oldDeviceConnected;
  unsigned long _lastNotifyTime;

  static const unsigned long NOTIFY_INTERVAL = 1000;
};

#endif // BLE_MANAGER_H
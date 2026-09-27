#include "GPSManager.h"

GPSManager::GPSManager() : _gpsSerial(2) {} // HardwareSerial 2 on ESP32

void GPSManager::begin(uint8_t rxPin, uint8_t txPin, uint32_t baudRate) {
  _gpsSerial.begin(baudRate, SERIAL_8N1, rxPin, txPin);
  Serial.printf("[GPS] Initialized UART at %u baud (RX: %u, TX: %u)\n", baudRate, rxPin, txPin);
}

void GPSManager::update() {
  while (_gpsSerial.available() > 0) {
    char c = _gpsSerial.read();
    _gps.encode(c);
  }

  // --- Location Data ---
  if (_gps.location.isValid()) {
    _data.latitude = _gps.location.lat();
    _data.longitude = _gps.location.lng();
    _data.fixValid = true;
  } else {
    _data.fixValid = false;
  }

  // --- Altitude Data ---
  if (_gps.altitude.isValid()) {
    _data.altitudeMeters = _gps.altitude.meters();
  }

  // --- Satellites Count ---
  if (_gps.satellites.isValid()) {
    _data.satellites = _gps.satellites.value();
  } else {
    _data.satellites = 0;
  }

  // --- GPS UTC Time ---
  if (_gps.time.isValid()) {
    _data.hour   = _gps.time.hour();
    _data.minute = _gps.time.minute();
    _data.second = _gps.time.second();
  } else {
    _data.hour   = 0;
    _data.minute = 0;
    _data.second = 0;
  }
}

GPSData GPSManager::getData() const {
  return _data;
}
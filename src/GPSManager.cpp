#include "GPSManager.h"

GPSManager::GPSManager() : _gpsSerial(1) {}

void GPSManager::begin(uint8_t rxPin, uint8_t txPin, uint32_t baudRate) {
  _gpsSerial.begin(baudRate, SERIAL_8N1, rxPin, txPin);
}

void GPSManager::update() {
  while (_gpsSerial.available() > 0) {
    _parser.encode(_gpsSerial.read());
  }

  _data.fixValid = _parser.location.isValid();
  if (_data.fixValid) {
    _data.latitude  = _parser.location.lat();
    _data.longitude = _parser.location.lng();
  } else {
    _data.latitude  = 0.0;
    _data.longitude = 0.0;
  }

  _data.satellites     = _parser.satellites.isValid() ? _parser.satellites.value() : 0;
  _data.altitudeMeters = _parser.altitude.isValid() ? _parser.altitude.meters() : 0.0;
  _data.speedKmh       = _parser.speed.isValid() ? _parser.speed.kmph() : 0.0;
}

GPSData GPSManager::getData() const {
  return _data;
}
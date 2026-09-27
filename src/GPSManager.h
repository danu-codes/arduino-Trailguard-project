#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

struct GPSData {
  float latitude = 0.0;
  float longitude = 0.0;
  float altitudeMeters = 0.0;
  float speedKmh = 0.0;       // <--- Added Speed field
  uint8_t satellites = 0;
  bool fixValid = false;
  uint8_t hour = 0;
  uint8_t minute = 0;
  uint8_t second = 0;
};

class GPSManager {
public:
  GPSManager();
  void begin(uint8_t rxPin, uint8_t txPin, uint32_t baudRate = 9600);
  void update();
  GPSData getData() const;

private:
  HardwareSerial _gpsSerial;
  TinyGPSPlus _gps;
  GPSData _data;
};

#endif // GPS_MANAGER_H
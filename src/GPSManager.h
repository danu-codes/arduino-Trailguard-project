#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS++.h>

struct GPSData {
  bool fixValid;
  double latitude;
  double longitude;
  double altitudeMeters;
  uint32_t satellites;
  float speedKmh;
};

class GPSManager {
public:
  GPSManager();
  void begin(uint8_t rxPin, uint8_t txPin, uint32_t baudRate = 9600);
  void update();
  GPSData getData() const;

private:
  HardwareSerial _gpsSerial;
  TinyGPSPlus _parser;
  GPSData _data;
};

#endif // GPS_MANAGER_H
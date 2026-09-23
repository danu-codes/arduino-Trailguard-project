#ifndef MOTION_MANAGER_H
#define MOTION_MANAGER_H

#include <Arduino.h>
#include <Wire.h>

struct MotionData {
  float accelX;
  float accelY;
  float accelZ;
  float pitch;
  float roll;
  float heading;
  uint32_t stepCount;
};

class MotionManager {
public:
  MotionManager();

  bool begin(int sdaPin = 8, int sclPin = 9, uint8_t i2cAddress = 0x68);
  void update();
  MotionData getData() const;

private:
  uint8_t _address;
  MotionData _data;
};

#endif // MOTION_MANAGER_H
#include "MotionManager.h"
#include <math.h>

MotionManager::MotionManager() : _address(0x68) {
  _data = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0};
}

bool MotionManager::begin(int sdaPin, int sclPin, uint8_t i2cAddress) {
  _address = i2cAddress;

  Wire.begin(sdaPin, sclPin);

  Wire.beginTransmission(_address);
  if (Wire.endTransmission() != 0) {
    return false;
  }

  // Wake up MPU6050 from sleep mode
  Wire.beginTransmission(_address);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  return true;
}

void MotionManager::update() {
  Wire.beginTransmission(_address);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(_address, (uint8_t)6);

  if (Wire.available() == 6) {
    int16_t rawX = (Wire.read() << 8) | Wire.read();
    int16_t rawY = (Wire.read() << 8) | Wire.read();
    int16_t rawZ = (Wire.read() << 8) | Wire.read();

    float ax = rawX / 16384.0f;
    float ay = rawY / 16384.0f;
    float az = rawZ / 16384.0f;

    _data.accelX = ax;
    _data.accelY = ay;
    _data.accelZ = az;

    // Calculate Pitch and Roll
    _data.pitch = atan2(ay, sqrt(ax * ax + az * az)) * 180.0 / M_PI;
    _data.roll  = atan2(-ax, az) * 180.0 / M_PI;

    // Raw calculated angle
    float calculatedAngle = atan2(ay, ax) * 180.0 / M_PI;
    if (calculatedAngle < 0) calculatedAngle += 360.0f;

    // Apply Low-Pass Smoothing Filter to prevent instant/wild jitter flipping
    if (_data.heading == 0.0f) {
      _data.heading = calculatedAngle;
    } else {
      // Smooth out sudden spikes (90% old heading, 10% new reading)
      float diff = calculatedAngle - _data.heading;
      if (diff > 180.0f) diff -= 360.0f;
      if (diff < -180.0f) diff += 360.0f;
      
      _data.heading += (diff * 0.1f);
      if (_data.heading < 0.0f) _data.heading += 360.0f;
      if (_data.heading >= 360.0f) _data.heading -= 360.0f;
    }
  }
}

MotionData MotionManager::getData() const {
  return _data;
}
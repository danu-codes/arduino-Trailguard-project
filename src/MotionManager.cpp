#include "MotionManager.h"

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

    _data.accelX = rawX / 16384.0f;
    _data.accelY = rawY / 16384.0f;
    _data.accelZ = rawZ / 16384.0f;

    _data.pitch = atan2(_data.accelY, sqrt(_data.accelX * _data.accelX + _data.accelZ * _data.accelZ)) * 180.0 / M_PI;
    _data.roll  = atan2(-_data.accelX, _data.accelZ) * 180.0 / M_PI;

    _data.heading = atan2(_data.accelY, _data.accelX) * 180.0 / M_PI;
    if (_data.heading < 0) _data.heading += 360.0f;
  }
}

MotionData MotionManager::getData() const {
  return _data;
}
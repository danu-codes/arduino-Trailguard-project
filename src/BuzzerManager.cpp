#include "BuzzerManager.h"

BuzzerManager::BuzzerManager(uint8_t pin) 
  : _pin(pin), _stopTime(0), _lastOffCourseBeep(0), _isPlaying(false) {}

void BuzzerManager::begin() {
  pinMode(_pin, OUTPUT);
  // ESP32 Core v3 API: Attach pin with default 12-bit resolution at 2000Hz
  ledcAttach(_pin, 2000, 12); 
  noTone();
}

void BuzzerManager::tone(uint16_t frequency, uint16_t durationMs) {
  // ESP32 Core v3 API: Pass the GPIO pin directly instead of channel
  ledcWriteTone(_pin, frequency); 
  _stopTime = millis() + durationMs;
  _isPlaying = true;
}

void BuzzerManager::noTone() {
  // Passing frequency 0 stops the tone
  ledcWriteTone(_pin, 0); 
  _isPlaying = false;
}

void BuzzerManager::update() {
  if (_isPlaying && millis() >= _stopTime) {
    noTone();
  }
}

void BuzzerManager::playClick() {
  tone(NOTE_C6, 40);
}

void BuzzerManager::playSuccess() {
  tone(NOTE_E5, 100);
  delay(110);
  tone(NOTE_G5, 150);
}

void BuzzerManager::playWarning() {
  tone(220, 150);
  delay(160);
  tone(180, 200);
}

void BuzzerManager::playArrivalAlert() {
  tone(NOTE_C5, 100); delay(110);
  tone(NOTE_E5, 100); delay(110);
  tone(NOTE_G5, 100); delay(110);
  tone(NOTE_C6, 250);
}

void BuzzerManager::playPowerOff() {
  tone(NOTE_G5, 120); delay(130);
  tone(NOTE_E5, 120); delay(130);
  tone(NOTE_C5, 250); delay(260);
  noTone();
}

void BuzzerManager::playOffCourseAlarm() {
  if (millis() - _lastOffCourseBeep > 1000) {
    _lastOffCourseBeep = millis();
    tone(NOTE_A5, 200);
  }
}
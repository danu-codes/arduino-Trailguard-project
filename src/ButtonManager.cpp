#include "ButtonManager.h"

ButtonManager::ButtonManager() {
  for (int i = 0; i < BTN_COUNT; i++) {
    _currentState[i] = false;
    _previousState[i] = false;
    _lastDebounceTime[i] = 0;
  }
}

void ButtonManager::begin(const uint8_t pins[BTN_COUNT]) {
  for (int i = 0; i < BTN_COUNT; i++) {
    _pins[i] = pins[i];
    pinMode(_pins[i], INPUT_PULLUP);
  }
}

void ButtonManager::update() {
  unsigned long now = millis();

  for (int i = 0; i < BTN_COUNT; i++) {
    _previousState[i] = _currentState[i];
    bool rawReading = (digitalRead(_pins[i]) == LOW);  // LOW = pressed

    if (rawReading != _currentState[i]) {
      if ((now - _lastDebounceTime[i]) > DEBOUNCE_DELAY) {
        _currentState[i] = rawReading;
        _lastDebounceTime[i] = now;
      }
    }
  }
}

bool ButtonManager::isPressed(ButtonID btn) {
  if (btn < 0 || btn >= BTN_COUNT) return false;
  return _currentState[btn];
}

bool ButtonManager::isJustPressed(ButtonID btn) {
  if (btn < 0 || btn >= BTN_COUNT) return false;
  return _currentState[btn] && !_previousState[btn];
}
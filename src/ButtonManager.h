#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

enum ButtonID {
  BTN_BACK = 0,
  BTN_MENU,
  BTN_UP,
  BTN_DOWN,
  BTN_START,
  BTN_RETURN,
  BTN_OK,
  BTN_COUNT
};

class ButtonManager {
public:
  ButtonManager();
  void begin(const uint8_t pins[BTN_COUNT]);
  void update();
  bool isPressed(ButtonID btn);
  bool isJustPressed(ButtonID btn);

private:
  uint8_t _pins[BTN_COUNT];
  bool _currentState[BTN_COUNT];
  bool _previousState[BTN_COUNT];
  unsigned long _lastDebounceTime[BTN_COUNT];
  static const unsigned long DEBOUNCE_DELAY = 50;  // ms
};

#endif  // BUTTON_MANAGER_H
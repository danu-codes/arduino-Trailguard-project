#ifndef BUZZER_MANAGER_H
#define BUZZER_MANAGER_H

#include <Arduino.h>

#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_C6  1047

class BuzzerManager {
public:
  BuzzerManager(uint8_t pin);

  void begin();
  void update();

  void playClick();
  void playSuccess();
  void playWarning();
  void playArrivalAlert();
  void playPowerOff();
  void playOffCourseAlarm();

  void tone(uint16_t frequency, uint16_t durationMs);
  void noTone();

private:
  uint8_t _pin;
  unsigned long _stopTime;
  unsigned long _lastOffCourseBeep;
  bool _isPlaying;
};

#endif // BUZZER_MANAGER_H
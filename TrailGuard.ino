#include <Arduino.h>
#include <Wire.h>

#include "src/PinDefinitions.h"
#include "src/DisplayManager.h"
#include "src/MotionManager.h"
#include "src/GPSManager.h"
#include "src/RouteManager.h"
#include "src/BuzzerManager.h"
#include "src/TempManager.h"
#include "src/BLEManager.h"

// --- Global Manager Instances ---
DisplayManager display;
MotionManager motion;
GPSManager gpsManager;
RouteManager routeManager;
BuzzerManager buzzer(BUZZER_PIN);
TempManager tempSensor(DHT_PIN, DHT11);
BLEManager bleManager;

// Timing Variables
unsigned long lastMenuBtnTime = 0;
unsigned long lastNavBtnTime = 0;
unsigned long lastActionBtnTime = 0;
const unsigned long DEBOUNCE_DELAY = 250;  // ms

// Independent Timers & Summary Counters
unsigned long recordingStartTime = 0;
unsigned long returnStartTime = 0;

uint32_t lastTripDurationSec = 0;
uint16_t lastTripPointsCount = 0;

uint32_t lastReturnDurationSec = 0;
uint16_t lastReturnPointsCount = 0;

// Battery Sensing Helper
uint8_t readBatteryPercentage() {
  uint32_t adcSum = 0;
  const uint8_t SAMPLES = 32;  // Take 32 samples to eliminate jitter

  for (uint8_t i = 0; i < SAMPLES; i++) {
    adcSum += analogRead(BATTERY_ADC_PIN);
    delayMicroseconds(50);
  }

  float rawADC = (float)adcSum / (float)SAMPLES;

  // Calculate voltage: Divider cuts voltage in half (100k / 100k)
  // ESP32 ADC max is 3.3V across 4095 steps
  float voltage = (rawADC / 4095.0f) * 2.0f * 3.3f;

  // Map LiPo range: 3.2V (0%) to 4.2V (100%)
  int pct = (int)(((voltage - 3.2f) / (4.2f - 3.2f)) * 100.0f);

  // Static moving average smoothing filter for display stability
  static float smoothedPct = -1.0f;
  if (smoothedPct < 0.0f) {
    smoothedPct = (float)pct;
  } else {
    smoothedPct = (smoothedPct * 0.9f) + ((float)pct * 0.1f);
  }

  return (uint8_t)constrain((int)smoothedPct, 0, 100);
}

// Helper to resume active screen state after menu closes
void resumeActiveScreen() {
  display.closeMenu();
  if (routeManager.isReturnMode()) {
    display.setScreen(SCREEN_RETURN);
  } else if (routeManager.isRecording()) {
    display.setScreen(SCREEN_RECORDING);
  } else {
    display.setScreen(SCREEN_HOME);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- TrailGuard S3 Handheld Initializing ---");

  // Status LEDs Setup
  pinMode(LED_GPS_PIN, OUTPUT);
  pinMode(LED_BLE_PIN, OUTPUT);
  pinMode(LED_BAT_PIN, OUTPUT);
  digitalWrite(LED_BAT_PIN, HIGH);

  // Keypad Pins Setup
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
  pinMode(BTN_OK_PIN, INPUT_PULLUP);
  pinMode(BTN_TRIP_PIN, INPUT_PULLUP);
  pinMode(BTN_RETURN_PIN, INPUT_PULLUP);
  pinMode(BTN_MENU_PIN, INPUT_PULLUP);
  pinMode(BTN_BACK_PIN, INPUT_PULLUP);

  // Buzzer
  buzzer.begin();
  buzzer.playSuccess();

  // I2C & Display
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  if (!display.begin(0x3C)) {
    Serial.println("[ERROR] OLED Display initialization failed!");
  } else {
    display.renderBootScreen();
    delay(1500);
  }

  // IMU
  if (!motion.begin(I2C_SDA_PIN, I2C_SCL_PIN, 0x68)) {
    Serial.println("[WARNING] IMU/Compass not detected at 0x68!");
  }

  // GPS & Sensors
  gpsManager.begin(GPS_RX_PIN, GPS_TX_PIN, 9600);
  tempSensor.begin();

  // Bluetooth Low Energy
  bleManager.begin("TrailGuard-S3");

  Serial.println("[SYSTEM] Boot complete. Ready for navigation.");
}

void loop() {
  // --- 1. Background Updates ---
  gpsManager.update();
  motion.update();
  buzzer.update();
  tempSensor.update();

  GPSData currentPos = gpsManager.getData();
  unsigned long now = millis();

  // BLE Update & Status LED indicators
  bleManager.update(currentPos);
  digitalWrite(LED_BLE_PIN, bleManager.isConnected() ? HIGH : LOW);
  digitalWrite(LED_GPS_PIN, currentPos.fixValid ? HIGH : LOW);

  // --- 2. Background Breadcrumb Logging & Nav Rules ---
  if (routeManager.isRecording() && currentPos.fixValid) {
    if (routeManager.addBreadcrumb(currentPos.latitude, currentPos.longitude)) {
      buzzer.playClick();
      Serial.printf("[LOG] Breadcrumb logged: %.5f, %.5f (Total: %d)\n",
                    currentPos.latitude, currentPos.longitude, routeManager.getPointCount());
    }
  }

  if (routeManager.isReturnMode() && currentPos.fixValid) {
    routeManager.updateNavigation(currentPos.latitude, currentPos.longitude);

    if (routeManager.isOffCourse(currentPos.latitude, currentPos.longitude)) {
      buzzer.playOffCourseAlarm();
    }
  }

  // --- 3. Keypad & Input Handling ---

  // A. MENU TOGGLE BUTTON
  if (digitalRead(BTN_MENU_PIN) == LOW && (now - lastMenuBtnTime > DEBOUNCE_DELAY)) {
    lastMenuBtnTime = now;
    buzzer.playClick();

    if (display.isMenuOpen()) {
      resumeActiveScreen();
    } else {
      display.toggleMenu();
    }
  }

  // B. BACK BUTTON
  if (digitalRead(BTN_BACK_PIN) == LOW && (now - lastNavBtnTime > DEBOUNCE_DELAY)) {
    lastNavBtnTime = now;
    buzzer.playClick();

    if (display.isMenuOpen()) {
      resumeActiveScreen();
    } else if (display.getCurrentScreen() != SCREEN_HOME && display.getCurrentScreen() != SCREEN_RETURN && display.getCurrentScreen() != SCREEN_RECORDING) {
      if (display.getCurrentScreen() == SCREEN_TRIP_SUMMARY || display.getCurrentScreen() == SCREEN_RETURN_SUMMARY) {
        display.setScreen(SCREEN_HOME);
      } else {
        display.toggleMenu();
      }
    } else {
      display.setScreen(SCREEN_HOME);
    }
  }

  // C. MENU NAVIGATION CONTROLS
  if (display.isMenuOpen()) {
    if (digitalRead(BTN_UP_PIN) == LOW && (now - lastNavBtnTime > DEBOUNCE_DELAY)) {
      lastNavBtnTime = now;
      buzzer.playClick();
      display.previousMenuOption();
    }

    if (digitalRead(BTN_DOWN_PIN) == LOW && (now - lastNavBtnTime > DEBOUNCE_DELAY)) {
      lastNavBtnTime = now;
      buzzer.playClick();
      display.nextMenuOption();
    }

    if (digitalRead(BTN_OK_PIN) == LOW && (now - lastActionBtnTime > DEBOUNCE_DELAY)) {
      lastActionBtnTime = now;
      buzzer.playClick();

      switch (display.getSelectedMenuOption()) {
        case MENU_OPTION_GPS:
          display.setScreen(SCREEN_GPS);
          break;
        case MENU_OPTION_IMU:
          display.setScreen(SCREEN_IMU);
          break;
        case MENU_OPTION_TEMP:
          display.setScreen(SCREEN_TEMP);
          break;
        case MENU_OPTION_SYSTEM:
          display.setScreen(SCREEN_SYSTEM);
          break;
        case MENU_OPTION_BATTERY:      
          display.setScreen(SCREEN_BATTERY);
          break;
      }
      display.closeMenu();
    }
  }
  // D. DIRECT CONTROLS
  else {
    // START / STOP TRIP BUTTON
    if (digitalRead(BTN_TRIP_PIN) == LOW && (now - lastActionBtnTime > DEBOUNCE_DELAY)) {
      lastActionBtnTime = now;
      if (!routeManager.isRecording()) {
        routeManager.startRecording();
        recordingStartTime = millis();
        buzzer.playSuccess();
        display.setScreen(SCREEN_RECORDING);
      } else {
        lastTripDurationSec = (millis() - recordingStartTime) / 1000;
        lastTripPointsCount = routeManager.getPointCount();
        routeManager.stopRecording();
        buzzer.playWarning();
        display.setScreen(SCREEN_TRIP_SUMMARY);
      }
    }

    // RETURN MODE BUTTON
    if (digitalRead(BTN_RETURN_PIN) == LOW && (now - lastActionBtnTime > DEBOUNCE_DELAY)) {
      lastActionBtnTime = now;

      if (!routeManager.isReturnMode()) {
        if (routeManager.getPointCount() > 0) {
          if (routeManager.isRecording()) {
            lastTripDurationSec = (millis() - recordingStartTime) / 1000;
            lastTripPointsCount = routeManager.getPointCount();
            routeManager.stopRecording();
          }

          routeManager.startReturnMode();
          returnStartTime = millis();
          buzzer.playArrivalAlert();
          display.setScreen(SCREEN_RETURN);
        } else {
          buzzer.playWarning();
          display.renderMessage("ERROR", "No Points Logged!", 1000);
        }
      } else {
        // Prevent accidental immediate stop if bounced or double-clicked within 1.5 seconds of starting
        if (millis() - returnStartTime > 1500) {
          lastReturnDurationSec = (millis() - returnStartTime) / 1000;
          lastReturnPointsCount = routeManager.getPointCount();
          routeManager.stopReturnMode();
          buzzer.playClick();
          display.setScreen(SCREEN_RETURN_SUMMARY);
        }
      }
    }
  }

  // --- 4. OLED Display Render Loop (10 Hz) ---
  static unsigned long lastRender = 0;
  if (now - lastRender >= 100) {
    lastRender = now;

    String currentTemp = tempSensor.getTempString();
    String currentHum = tempSensor.getHumidityString();
    bool offCourseState = routeManager.isOffCourse(currentPos.latitude, currentPos.longitude);

    if (display.isMenuOpen()) {
      display.renderMenu();
    } else {
      if (offCourseState && routeManager.isReturnMode() && display.getCurrentScreen() != SCREEN_RETURN) {
        display.setScreen(SCREEN_RETURN);
      }

      switch (display.getCurrentScreen()) {
        case SCREEN_HOME:
          {
            uint8_t batPct = readBatteryPercentage();
            display.renderHome(currentPos, currentTemp, currentHum, bleManager.isConnected(), batPct);
            break;
          }

        case SCREEN_RECORDING:
          {
            uint32_t elapsedSec = (millis() - recordingStartTime) / 1000;
            uint8_t batPct = readBatteryPercentage();
            display.renderRecording(currentPos, elapsedSec, routeManager.getPointCount(), batPct);
            break;
          }

        case SCREEN_TRIP_SUMMARY:
          {
            uint8_t batPct = readBatteryPercentage();
            display.renderTripSummary(lastTripDurationSec, lastTripPointsCount, batPct);
            break;
          }

        case SCREEN_RETURN_SUMMARY:
          {
            uint8_t batPct = readBatteryPercentage();
            display.renderReturnSummary(lastReturnDurationSec, lastReturnPointsCount, batPct);
            break;
          }

        case SCREEN_GPS:
          display.renderGPS(currentPos);
          break;

        case SCREEN_IMU:
          display.renderIMU(motion.getData());
          break;

        case SCREEN_TEMP:
          display.renderTempScreen(currentTemp, currentHum);
          break;

        case SCREEN_SYSTEM:
          {
            uint8_t batPct = readBatteryPercentage();
            display.renderSystemInfo(batPct);
            break;
          }

          case SCREEN_BATTERY:            
          {
            uint8_t batPct = readBatteryPercentage();
            display.renderBatteryScreen(batPct);
            break;
          }

        case SCREEN_RETURN:
          {
            uint32_t elapsedReturnSec = (millis() - returnStartTime) / 1000;
            uint8_t batPct = readBatteryPercentage();
            display.renderReturnNav(
              routeManager.getDistanceToNext(currentPos.latitude, currentPos.longitude),
              routeManager.getBearingToNext(currentPos.latitude, currentPos.longitude),
              motion.getData().heading,
              routeManager.getCurrentTargetIndex() + 1,
              routeManager.getPointCount(),
              offCourseState,
              elapsedReturnSec,
              batPct);
            break;
          }
      }
    }
  }
}
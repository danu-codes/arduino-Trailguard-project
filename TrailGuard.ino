#include <Arduino.h>
#include <Wire.h>

#include "src/PinDefinitions.h"
#include "src/DisplayManager.h"
#include "src/MotionManager.h"
#include "src/GPSManager.h"
#include "src/RouteManager.h"
#include "src/BuzzerManager.h"
#include "src/TempManager.h"

// --- Global Manager Instances ---
DisplayManager display;
MotionManager  motion;
GPSManager     gpsManager;
RouteManager   routeManager;
BuzzerManager  buzzer(BUZZER_PIN);
TempManager    tempSensor(DHT_PIN, DHT22);

// --- Button Timing & Power Variables ---
unsigned long powerBtnPressStart = 0;
bool powerBtnPressed = false;

// Debounce helpers
unsigned long lastNavBtnTime = 0;
unsigned long lastActionBtnTime = 0;
const unsigned long DEBOUNCE_DELAY = 200; // ms

// Helper to resume active state when menu is exited
void resumeActiveScreen() {
  display.closeMenu();
  if (routeManager.isReturnMode()) {
    display.setScreen(SCREEN_RETURN);
  } else {
    display.setScreen(SCREEN_GPS);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n--- TrailGuard S3 Handheld Initializing ---");

  // Keypad Pins
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
  pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
  pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BTN_OK_PIN, INPUT_PULLUP);
  pinMode(BTN_START_PIN, INPUT_PULLUP);
  pinMode(BTN_POWER_PIN, INPUT_PULLUP);
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

  Serial.println("[SYSTEM] Boot complete. Ready for navigation.");
}

void loop() {
  // --- 1. Background Updates (Always run regardless of Menu) ---
  gpsManager.update();
  motion.update();
  buzzer.update();
  tempSensor.update();

  GPSData currentPos = gpsManager.getData();
  unsigned long now = millis();

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

  // A. MENU TOGGLE (Menu button opens/closes menu)
  if (digitalRead(BTN_MENU_PIN) == LOW && (now - lastNavBtnTime > DEBOUNCE_DELAY)) {
    lastNavBtnTime = now;
    buzzer.playClick();
    if (display.isMenuOpen()) {
      resumeActiveScreen(); // Resume previous state
    } else {
      display.toggleMenu(); // Open menu overlay
    }
  }

  // B. BACK BUTTON (Only works inside Menu)
  if (digitalRead(BTN_BACK_PIN) == LOW && (now - lastNavBtnTime > DEBOUNCE_DELAY)) {
    lastNavBtnTime = now;
    buzzer.playClick();
    if (display.isMenuOpen()) {
      resumeActiveScreen(); // Exit menu back to active state
    }
  }

  // C. MENU NAVIGATION CONTROLS (Active while Menu is Open)
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

      // Open selected menu screen and exit menu overlay
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
      }
      display.closeMenu();
    }
  } 
  // D. NORMAL NAVIGATION CONTROLS (Active when Menu is Closed)
  else {
    // Quick Left/Right screen cycle
    if (digitalRead(BTN_RIGHT_PIN) == LOW && (now - lastNavBtnTime > DEBOUNCE_DELAY)) {
      lastNavBtnTime = now;
      buzzer.playClick();
      display.nextScreen();
    }
    if (digitalRead(BTN_LEFT_PIN) == LOW && (now - lastNavBtnTime > DEBOUNCE_DELAY)) {
      lastNavBtnTime = now;
      buzzer.playClick();
      display.previousScreen();
    }

    // START/STOP Trail Recording
    if (digitalRead(BTN_START_PIN) == LOW && (now - lastActionBtnTime > DEBOUNCE_DELAY)) {
      lastActionBtnTime = now;
      if (!routeManager.isRecording()) {
        routeManager.startRecording();
        buzzer.playSuccess();
        display.renderMessage("TRAIL TRACKER", "Recording Started!", 1000);
      } else {
        routeManager.stopRecording();
        buzzer.playWarning();
        display.renderMessage("TRAIL TRACKER", "Recording Stopped.", 1000);
      }
    }

    // TOGGLE Backtrack Navigation Mode
    if (digitalRead(BTN_OK_PIN) == LOW && (now - lastActionBtnTime > DEBOUNCE_DELAY)) {
      lastActionBtnTime = now;
      if (!routeManager.isReturnMode()) {
        if (routeManager.getPointCount() > 0) {
          routeManager.startReturnMode();
          buzzer.playArrivalAlert();
          display.setScreen(SCREEN_RETURN);
          display.renderMessage("BACKTRACK NAV", "Return Mode Active!", 1000);
        } else {
          buzzer.playWarning();
          display.renderMessage("ERROR", "No Route Points Logged!", 1000);
        }
      } else {
        routeManager.stopReturnMode();
        buzzer.playClick();
        display.renderMessage("BACKTRACK NAV", "Return Mode Stopped.", 1000);
      }
    }
  }

  // E. POWER OFF / DEEP SLEEP (Hold Power for 2 Seconds)
  if (digitalRead(BTN_POWER_PIN) == LOW) {
    if (!powerBtnPressed) {
      powerBtnPressed = true;
      powerBtnPressStart = now;
    } else if (now - powerBtnPressStart >= 2000) {
      buzzer.playPowerOff();
      display.renderMessage("POWER OFF", "Shutting down...", 1000);

      esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_POWER_PIN, LOW);
      esp_deep_sleep_start();
    }
  } else {
    powerBtnPressed = false;
  }

  // --- 4. OLED Display Render Loop (10 Hz) ---
  static unsigned long lastRender = 0;
  if (now - lastRender >= 100) {
    lastRender = now;

    String currentTemp = tempSensor.getTempString();
    String currentHum  = tempSensor.getHumidityString();
    bool offCourseState = routeManager.isOffCourse(currentPos.latitude, currentPos.longitude);

    // If Menu is active, draw Menu overlay
    if (display.isMenuOpen()) {
      display.renderMenu();
    } 
    // Otherwise render active screen
    else {
      // Auto switch back to Return screen if off-course alert fires during Return Mode
      if (offCourseState && routeManager.isReturnMode() && display.getCurrentScreen() != SCREEN_RETURN) {
        display.setScreen(SCREEN_RETURN);
      }

      switch (display.getCurrentScreen()) {
        case SCREEN_GPS:
          display.renderGPS(currentPos, currentTemp, currentHum);
          break;

        case SCREEN_IMU:
          display.renderIMU(motion.getData(), currentTemp, currentHum);
          break;

        case SCREEN_TEMP:
          display.renderTempScreen(currentTemp, currentHum);
          break;

        case SCREEN_SYSTEM:
          display.renderSystemInfo(currentTemp, currentHum);
          break;

        case SCREEN_RETURN:
          display.renderReturnNav(
            routeManager.getDistanceToNext(currentPos.latitude, currentPos.longitude),
            routeManager.getBearingToNext(currentPos.latitude, currentPos.longitude),
            motion.getData().heading,
            routeManager.getCurrentTargetIndex() + 1,
            routeManager.getPointCount(),
            currentTemp,
            currentHum,
            offCourseState
          );
          break;
      }
    }
  }
}
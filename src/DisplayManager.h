#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "GPSManager.h"
#include "MotionManager.h"

enum ScreenState {
  SCREEN_HOME,
  SCREEN_GPS,
  SCREEN_IMU,
  SCREEN_TEMP,
  SCREEN_SYSTEM,
  SCREEN_RETURN,
  SCREEN_RECORDING,
  SCREEN_TRIP_SUMMARY,    // <--- Trip Summary View
  SCREEN_RETURN_SUMMARY   // <--- Return Summary View
};

enum MenuOption {
  MENU_OPTION_GPS,
  MENU_OPTION_IMU,
  MENU_OPTION_TEMP,
  MENU_OPTION_SYSTEM,
  MENU_OPTION_COUNT
};

class DisplayManager {
public:
  DisplayManager();
  bool begin(uint8_t i2cAddress = 0x3C);

  void setScreen(ScreenState screen);
  ScreenState getCurrentScreen() const;
  void nextScreen();
  void previousScreen();

  // Menu methods
  void toggleMenu();
  bool isMenuOpen() const;
  void closeMenu();
  void nextMenuOption();
  void previousMenuOption();
  MenuOption getSelectedMenuOption() const;

  // Render methods
  void renderBootScreen();
  void renderMessage(const String& title, const String& msg, uint16_t delayMs = 0);
  void renderMenu();
  
  // Render Screens
  void renderHome(const GPSData& gps, const String& temp, const String& hum, bool bleConnected);
  void renderGPS(const GPSData& gps, const String& temp, const String& hum);
  void renderIMU(const MotionData& imu, const String& temp, const String& hum);
  void renderTempScreen(const String& temp, const String& hum);
  void renderSystemInfo(const String& temp, const String& hum);
  
  void renderReturnNav(double distance, double bearing, double heading,
                       uint16_t currentIdx, uint16_t totalIdx,
                       const String& temp, const String& hum,
                       bool isOffCourse, uint32_t elapsedTimeSec, uint8_t batPct);

  void renderRecording(const GPSData& gps, uint32_t elapsedTimeSec, uint16_t pointCount, uint8_t batPct);

  // Summary Screen Renderers
  void renderTripSummary(uint32_t totalTimeSec, uint16_t totalPts, uint8_t batPct);
  void renderReturnSummary(uint32_t totalTimeSec, uint16_t ptsReturned, uint8_t batPct);

private:
  Adafruit_SH1106G _display;
  ScreenState _currentScreen;
  bool _inMenu;
  uint8_t _menuIndex;

  void drawHeader(const char* title, const String& temp, const String& hum);
  void drawTurnArrow(int16_t cx, int16_t cy, float relativeAngle);
  void drawOffCourseBanner();
};

#endif  // DISPLAY_MANAGER_H
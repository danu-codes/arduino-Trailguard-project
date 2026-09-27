#include "DisplayManager.h"
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DisplayManager::DisplayManager()
  : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
    _currentScreen(SCREEN_HOME),
    _inMenu(false),
    _menuIndex(0) {}

bool DisplayManager::begin(uint8_t i2cAddress) {
  if (!_display.begin(i2cAddress, true)) {
    return false;
  }
  _display.clearDisplay();
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setTextSize(1);
  _display.display();
  return true;
}

void DisplayManager::setScreen(ScreenState screen) {
  _currentScreen = screen;
}

ScreenState DisplayManager::getCurrentScreen() const {
  return _currentScreen;
}

void DisplayManager::nextScreen() {
  _currentScreen = static_cast<ScreenState>((_currentScreen + 1) % 9);
}

void DisplayManager::previousScreen() {
  _currentScreen = static_cast<ScreenState>((_currentScreen + 8) % 9);
}

void DisplayManager::toggleMenu() { _inMenu = !_inMenu; }
bool DisplayManager::isMenuOpen() const { return _inMenu; }
void DisplayManager::closeMenu() { _inMenu = false; }
void DisplayManager::nextMenuOption() { _menuIndex = (_menuIndex + 1) % MENU_OPTION_COUNT; }
void DisplayManager::previousMenuOption() { _menuIndex = (_menuIndex + MENU_OPTION_COUNT - 1) % MENU_OPTION_COUNT; }
MenuOption DisplayManager::getSelectedMenuOption() const { return static_cast<MenuOption>(_menuIndex); }

void DisplayManager::drawHeader(const char* title, const String& temp, const String& hum) {
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.print(title);
  _display.setCursor(85, 0);
  _display.print(temp);
  _display.drawFastHLine(0, 10, 128, SH110X_WHITE);
}

void DisplayManager::renderMenu() {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setCursor(35, 0);
  _display.print("= MENU =");
  _display.drawFastHLine(0, 10, 128, SH110X_WHITE);

  const char* options[] = {
    "1. GPS Coordinates",
    "2. MPU Sensor Data",
    "3. Temp & Humidity",
    "4. System Info"
  };

  for (uint8_t i = 0; i < MENU_OPTION_COUNT; i++) {
    uint8_t y = 14 + (i * 12);
    if (i == _menuIndex) {
      _display.fillRect(0, y - 1, 128, 11, SH110X_WHITE);
      _display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    } else {
      _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
    }
    _display.setCursor(4, y);
    _display.print(options[i]);
  }

  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.display();
}

void DisplayManager::renderRecording(const GPSData& gps, uint32_t elapsedTimeSec, uint16_t pointCount, uint8_t batPct) {
  _display.clearDisplay();
  
  _display.setTextSize(1);
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setCursor(0, 0);
  
  if ((millis() / 500) % 2 == 0) {
    _display.print("[REC]");
  } else {
    _display.print("[   ]");
  }

  _display.printf(" TRAIL  BAT:%u%%", batPct);
  _display.drawFastHLine(0, 10, 128, SH110X_WHITE);

  uint32_t hrs = elapsedTimeSec / 3600;
  uint32_t mins = (elapsedTimeSec % 3600) / 60;
  uint32_t secs = elapsedTimeSec % 60;

  _display.setCursor(0, 15);
  _display.printf("TIME: %02u:%02u:%02u", hrs, mins, secs);

  _display.setCursor(0, 27);
  _display.printf("SPD : %.1f km/h", gps.speedKmh);

  _display.setCursor(0, 39);
  _display.printf("PTS : %u / 300", pointCount);

  _display.setCursor(0, 51);
  if (gps.fixValid) {
    _display.printf("GPS : LOCK (%u SAT)", gps.satellites);
  } else {
    _display.printf("GPS : SEARCHING(%u)", gps.satellites);
  }

  _display.display();
}

// --- TRIP SUMMARY SCREEN ---
void DisplayManager::renderTripSummary(uint32_t totalTimeSec, uint16_t totalPts, uint8_t batPct) {
  _display.clearDisplay();
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setTextSize(1);
  _display.setCursor(15, 0);
  _display.print("= TRIP SUMMARY =");
  _display.drawFastHLine(0, 10, 128, SH110X_WHITE);

  uint32_t hrs = totalTimeSec / 3600;
  uint32_t mins = (totalTimeSec % 3600) / 60;
  uint32_t secs = totalTimeSec % 60;

  _display.setCursor(0, 16);
  _display.printf("Duration : %02u:%02u:%02u\n", hrs, mins, secs);
  _display.printf("Logged Pts: %u pts\n", totalPts);
  _display.printf("Battery   : %u%%\n", batPct);
  
  _display.setCursor(0, 52);
  _display.print("[Press BACK -> Home]");
  _display.display();
}

// --- RETURN NAV SUMMARY SCREEN ---
void DisplayManager::renderReturnSummary(uint32_t totalTimeSec, uint16_t ptsReturned, uint8_t batPct) {
  _display.clearDisplay();
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setTextSize(1);
  _display.setCursor(10, 0);
  _display.print("= RETURN SUMMARY =");
  _display.drawFastHLine(0, 10, 128, SH110X_WHITE);

  uint32_t hrs = totalTimeSec / 3600;
  uint32_t mins = (totalTimeSec % 3600) / 60;
  uint32_t secs = totalTimeSec % 60;

  _display.setCursor(0, 16);
  _display.printf("Return Time: %02u:%02u:%02u\n", hrs, mins, secs);
  _display.printf("Nodes Nav  : %u pts\n", ptsReturned);
  _display.printf("Battery    : %u%%\n", batPct);

  _display.setCursor(0, 52);
  _display.print("[Press BACK -> Home]");
  _display.display();
}

void DisplayManager::renderHome(const GPSData& gps, const String& temp, const String& hum, bool bleConnected) {
  _display.clearDisplay();
  _display.setTextColor(SH110X_WHITE);

  _display.setTextSize(2);
  _display.setCursor(16, 2);
  if (gps.fixValid) {
    char timeBuffer[10];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", gps.hour, gps.minute, gps.second);
    _display.print(timeBuffer);
  } else {
    _display.print("--:--:--");
  }

  _display.drawLine(0, 20, 128, 20, SH110X_WHITE);

  _display.setTextSize(1);
  _display.setCursor(0, 24);
  _display.printf("TMP:%s | HUM:%s", temp.c_str(), hum.c_str());

  _display.setCursor(0, 38);
  if (gps.fixValid) {
    _display.printf("SAT: %02d (3D LOCK)", gps.satellites);
  } else {
    _display.printf("SAT: %02d (SEARCHING)", gps.satellites);
  }

  _display.setCursor(0, 52);
  if (bleConnected) {
    _display.print("BLE: APP CONNECTED");
  } else {
    _display.print("BLE: DISCONNECTED");
  }

  _display.display();
}

void DisplayManager::renderGPS(const GPSData& gps, const String& temp, const String& hum) {
  _display.clearDisplay();
  drawHeader("GPS COORDINATES", temp, hum);

  _display.setCursor(0, 16);
  _display.setTextSize(1);
  if (gps.fixValid) {
    _display.printf("Lat: %.5f\n", gps.latitude);
    _display.printf("Lng: %.5f\n", gps.longitude);
    _display.printf("Alt: %.1fm\n", gps.altitudeMeters);
    _display.printf("Spd: %.1fkm/h\n", gps.speedKmh);
  } else {
    _display.println("Searching Signal...");
    _display.printf("Sats Visible: %d\n", gps.satellites);
  }
  _display.display();
}

void DisplayManager::renderIMU(const MotionData& imu, const String& temp, const String& hum) {
  _display.clearDisplay();
  drawHeader("COMPASS / IMU", temp, hum);

  _display.setCursor(0, 16);
  _display.print("Heading: "); _display.print(imu.heading, 1); _display.println(" deg");
  _display.print("Pitch:   "); _display.print(imu.pitch, 1); _display.println(" deg");
  _display.print("Roll:    "); _display.print(imu.roll, 1); _display.println(" deg");
  _display.print("Steps:   "); _display.println(imu.stepCount);
  _display.display();
}

void DisplayManager::renderTempScreen(const String& temp, const String& hum) {
  _display.clearDisplay();
  drawHeader("ENVIRONMENT", temp, hum);

  _display.setCursor(0, 20);
  _display.setTextSize(1);
  _display.print("Temperature : "); _display.println(temp);
  _display.println();
  _display.print("Humidity    : "); _display.println(hum);
  _display.display();
}

void DisplayManager::renderSystemInfo(const String& temp, const String& hum) {
  _display.clearDisplay();
  drawHeader("SYSTEM STATUS", temp, hum);

  _display.setCursor(0, 16);
  _display.print("Temp: "); _display.println(temp);
  _display.print("Hum:  "); _display.println(hum);
  _display.print("Heap: "); _display.print(ESP.getFreeHeap() / 1024); _display.println(" KB");
  _display.display();
}

void DisplayManager::drawOffCourseBanner() {
  if ((millis() / 500) % 2 == 0) {
    _display.fillRect(0, 12, 128, 12, SH110X_WHITE);
    _display.setTextColor(SH110X_BLACK, SH110X_WHITE);
    _display.setTextSize(1);
    _display.setCursor(18, 14);
    _display.print("! OFF COURSE !");
    _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  }
}

void DisplayManager::drawTurnArrow(int16_t cx, int16_t cy, float relativeAngle) {
  while (relativeAngle > 180.0f) relativeAngle -= 360.0f;
  while (relativeAngle < -180.0f) relativeAngle += 360.0f;

  _display.drawCircle(cx, cy, 14, SH110X_WHITE);

  if (relativeAngle > 45.0f && relativeAngle <= 135.0f) {
    _display.fillTriangle(cx + 6, cy, cx + 2, cy - 4, cx + 2, cy + 4, SH110X_WHITE);
    _display.drawLine(cx - 6, cy + 4, cx + 2, cy + 4, SH110X_WHITE);
    _display.drawLine(cx - 6, cy - 2, cx - 6, cy + 4, SH110X_WHITE);
  } else if (relativeAngle > 135.0f || relativeAngle < -135.0f) {
    _display.fillTriangle(cx - 6, cy, cx - 2, cy - 4, cx - 2, cy + 4, SH110X_WHITE);
    _display.drawLine(cx - 6, cy + 4, cx + 6, cy + 4, SH110X_WHITE);
    _display.drawLine(cx + 6, cy - 2, cx + 6, cy + 4, SH110X_WHITE);
  } else if (relativeAngle < -45.0f && relativeAngle >= -135.0f) {
    _display.fillTriangle(cx - 6, cy, cx - 2, cy - 4, cx - 2, cy + 4, SH110X_WHITE);
    _display.drawLine(cx - 2, cy + 4, cx + 6, cy + 4, SH110X_WHITE);
    _display.drawLine(cx + 6, cy - 2, cx + 6, cy + 4, SH110X_WHITE);
  } else {
    _display.fillTriangle(cx, cy - 8, cx - 4, cy - 2, cx + 4, cy - 2, SH110X_WHITE);
    _display.drawLine(cx, cy - 2, cx, cy + 8, SH110X_WHITE);
  }
}

void DisplayManager::renderBootScreen() {
  _display.clearDisplay();
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setTextSize(2);
  _display.setCursor(5, 15);
  _display.print("TRAILGUARD");
  _display.setTextSize(1);
  _display.setCursor(20, 42);
  _display.print("S3 Handheld GPS");
  _display.display();
}

void DisplayManager::renderMessage(const String& title, const String& msg, uint16_t delayMs) {
  _display.clearDisplay();
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setTextSize(1);
  _display.setCursor(0, 10);
  _display.println(title);
  _display.drawFastHLine(0, 22, 128, SH110X_WHITE);
  _display.setCursor(0, 32);
  _display.println(msg);
  _display.display();
  if (delayMs > 0) {
    delay(delayMs);
  }
}

void DisplayManager::renderReturnNav(double distance, double bearing, double heading,
                                     uint16_t currentIdx, uint16_t totalIdx,
                                     const String& temp, const String& hum,
                                     bool isOffCourse, uint32_t elapsedTimeSec, uint8_t batPct) {
  _display.clearDisplay();
  
  _display.setTextSize(1);
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setCursor(0, 0);
  _display.print("BACKTRACK  BAT:");
  _display.print(batPct);
  _display.print("%");
  _display.drawFastHLine(0, 10, 128, SH110X_WHITE);

  if (isOffCourse) {
    drawOffCourseBanner();
  }

  float relativeAngle = bearing - heading;
  while (relativeAngle > 180.0f) relativeAngle -= 360.0f;
  while (relativeAngle < -180.0f) relativeAngle += 360.0f;

  drawTurnArrow(105, 38, relativeAngle);

  uint8_t startY = isOffCourse ? 26 : 14;

  _display.setCursor(0, startY);
  _display.setTextSize(1);
  if (relativeAngle > 45.0f && relativeAngle <= 135.0f) {
    _display.print("-> GO RIGHT");
  } else if (relativeAngle > 135.0f || relativeAngle < -135.0f) {
    _display.print("<- U-TURN BACK");
  } else if (relativeAngle < -45.0f && relativeAngle >= -135.0f) {
    _display.print("<- GO LEFT");
  } else {
    _display.print("^ KEEP STRAIGHT");
  }

  _display.setCursor(0, startY + 13);
  _display.setTextSize(1);
  _display.printf("Dist: %.1fm", distance);

  uint32_t hrs = elapsedTimeSec / 3600;
  uint32_t mins = (elapsedTimeSec % 3600) / 60;
  uint32_t secs = elapsedTimeSec % 60;
  _display.setCursor(0, startY + 25);
  _display.printf("Time: %02u:%02u:%02u", hrs, mins, secs);

  _display.setCursor(0, startY + 37);
  _display.printf("Node: %u/%u", currentIdx, totalIdx);

  _display.display();
}
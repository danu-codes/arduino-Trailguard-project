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

void DisplayManager::toggleMenu() { _inMenu = !_inMenu; }
bool DisplayManager::isMenuOpen() const { return _inMenu; }
void DisplayManager::closeMenu() { _inMenu = false; }
void DisplayManager::nextMenuOption() { _menuIndex = (_menuIndex + 1) % MENU_OPTION_COUNT; }
void DisplayManager::previousMenuOption() { _menuIndex = (_menuIndex + MENU_OPTION_COUNT - 1) % MENU_OPTION_COUNT; }
MenuOption DisplayManager::getSelectedMenuOption() const { return static_cast<MenuOption>(_menuIndex); }

void DisplayManager::drawHeader(const char* title, const char* rightText) {
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.print(title);
  
  if (rightText != nullptr && strlen(rightText) > 0) {
    int16_t x = 128 - (strlen(rightText) * 6);
    if (x > 50) {
      _display.setCursor(x, 0);
      _display.print(rightText);
    }
  }
  _display.drawFastHLine(0, 10, 128, SH110X_WHITE);
}

void DisplayManager::renderMenu() {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setCursor(38, 0);
  _display.print("= MENU =");
  _display.drawFastHLine(0, 10, 128, SH110X_WHITE);

  const char* options[] = {
    "1. GPS Coordinates",
    "2. IMU / Compass",
    "3. Environment",
    "4. System Status",
    "5. Battery Details"
  };

  // Adjusted spacing to fit all 5 options within the 64px height limit
  for (uint8_t i = 0; i < MENU_OPTION_COUNT; i++) {
    uint8_t y = 13 + (i * 10); // Spacing reduced from 12 to 10 pixels
    if (i == _menuIndex) {
      _display.fillRect(0, y - 1, 128, 9, SH110X_WHITE); // Highlight bar height adjusted to 9
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

void DisplayManager::renderHome(const GPSData& gps, const String& temp, const String& hum, bool bleConnected, uint8_t batPct) {
  _display.clearDisplay();
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);

  // --- Top Bar: Status / Battery ---
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.print("TRAILGUARD");
  
  _display.setCursor(80, 0);
  _display.printf("BAT:%3u%%", batPct);
  _display.drawFastHLine(0, 9, 128, SH110X_WHITE);

  // --- Main Time Display ---
  _display.setTextSize(2);
  _display.setCursor(16, 14);
  
  // Displays GPS time if fixed; otherwise shows fallback clock format
  if (gps.fixValid || (gps.hour > 0 || gps.minute > 0 || gps.second > 0)) {
    char timeBuffer[10];
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d", gps.hour, gps.minute, gps.second);
    _display.print(timeBuffer);
  } else {
    _display.print("--:--:--");
  }

  _display.drawFastHLine(0, 32, 128, SH110X_WHITE);

  // --- Sensors & Connections ---
  _display.setTextSize(1);
  _display.setCursor(0, 36);
  _display.printf("TMP:%s | HUM:%s", temp.c_str(), hum.c_str());

  _display.setCursor(0, 46);
  if (gps.fixValid) {
    _display.printf("GPS: 3D LOCK (%d SAT)", gps.satellites);
  } else {
    _display.printf("GPS: SEARCHING (%d)", gps.satellites);
  }

  _display.setCursor(0, 56);
  _display.print(bleConnected ? "BLE: CONNECTED" : "BLE: DISCONNECTED");

  _display.display();
}

void DisplayManager::renderGPS(const GPSData& gps) {
  _display.clearDisplay();
  drawHeader("GPS COORDINATES", gps.fixValid ? "LOCK" : "NO FIX");

  _display.setCursor(0, 16);
  _display.setTextSize(1);
  if (gps.fixValid) {
    _display.printf("Lat : %.5f\n", gps.latitude);
    _display.printf("Lng : %.5f\n", gps.longitude);
    _display.printf("Alt : %.1f m\n", gps.altitudeMeters);
    _display.printf("Spd : %.1f km/h\n", gps.speedKmh);
    _display.printf("Sats: %u satellites", gps.satellites);
  } else {
    _display.println("Searching GPS Signal...");
    _display.println();
    _display.printf("Sats Visible: %u\n", gps.satellites);
    _display.println("Acquiring 3D Lock...");
  }
  _display.display();
}

void DisplayManager::renderIMU(const MotionData& imu) {
  _display.clearDisplay();
  drawHeader("COMPASS / IMU", "");

  _display.setCursor(0, 16);
  _display.setTextSize(1);
  _display.printf("Heading: %.1f deg\n", imu.heading);
  _display.printf("Pitch  : %.1f deg\n", imu.pitch);
  _display.printf("Roll   : %.1f deg\n", imu.roll);
  _display.printf("Steps  : %u\n", imu.stepCount);
  _display.display();
}

void DisplayManager::renderTempScreen(const String& temp, const String& hum) {
  _display.clearDisplay();
  drawHeader("ENVIRONMENT", "");

  _display.setCursor(0, 20);
  _display.setTextSize(1);
  _display.printf("Temperature : %s\n\n", temp.c_str());
  _display.printf("Humidity    : %s\n", hum.c_str());
  _display.display();
}

void DisplayManager::renderSystemInfo(uint8_t batPct) {
  _display.clearDisplay();
  drawHeader("SYSTEM STATUS", "");

  _display.setCursor(0, 16);
  _display.setTextSize(1);

  uint32_t freeHeapKb = ESP.getFreeHeap() / 1024;
  uint32_t minHeapKb  = ESP.getMinFreeHeap() / 1024;
  uint32_t maxAllocKb = ESP.getMaxAllocHeap() / 1024;

  _display.printf("Free Heap : %u KB\n", freeHeapKb);
  _display.printf("Min Heap  : %u KB\n", minHeapKb);
  _display.printf("Max Block : %u KB\n", maxAllocKb);
  _display.printf("Battery   : %u%%\n", batPct);
  _display.printf("CPU Freq  : %u MHz", ESP.getCpuFreqMHz());

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

  _display.setCursor(0, 16);
  _display.printf("TIME: %02u:%02u:%02u\n", hrs, mins, secs);
  _display.printf("SPD : %.1f km/h\n", gps.speedKmh);
  _display.printf("PTS : %u / 300\n", pointCount);

  if (gps.fixValid) {
    _display.printf("GPS : LOCK (%u SAT)", gps.satellites);
  } else {
    _display.printf("GPS : SEARCHING(%u)", gps.satellites);
  }

  _display.display();
}

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
                                     bool isOffCourse, uint32_t elapsedTimeSec, uint8_t batPct) {
  _display.clearDisplay();
  
  _display.setTextSize(1);
  _display.setTextColor(SH110X_WHITE, SH110X_BLACK);
  _display.setCursor(0, 0);
  
  if (isOffCourse && ((millis() / 500) % 2 == 0)) {
    _display.print("!OFF-COURSE!");
  } else {
    _display.print("BACKTRACK ");
  }

  _display.printf(" BAT:%u%%", batPct);
  _display.drawFastHLine(0, 10, 128, SH110X_WHITE);

  float relativeAngle = bearing - heading;
  while (relativeAngle > 180.0f) relativeAngle -= 360.0f;
  while (relativeAngle < -180.0f) relativeAngle += 360.0f;

  drawTurnArrow(105, 38, relativeAngle);

  _display.setCursor(0, 14);
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

  _display.setCursor(0, 27);
  _display.printf("Dist: %.1fm", distance);

  uint32_t hrs = elapsedTimeSec / 3600;
  uint32_t mins = (elapsedTimeSec % 3600) / 60;
  uint32_t secs = elapsedTimeSec % 60;
  _display.setCursor(0, 39);
  _display.printf("Time: %02u:%02u:%02u", hrs, mins, secs);

  _display.setCursor(0, 51);
  _display.printf("Node: %u/%u", currentIdx, totalIdx);

  _display.display();
}

void DisplayManager::renderBatteryScreen(uint8_t batPct) {
  _display.clearDisplay();
  drawHeader("BATTERY DETAILS", "");

  _display.setCursor(0, 18);
  _display.setTextSize(1);
  _display.printf("Status  : %s\n", batPct > 20 ? "NORMAL" : "LOW BATTERY!");
  _display.printf("Level   : %u%%\n\n", batPct);

  // Draw a visual battery container box on the OLED
  _display.drawRect(14, 40, 100, 16, SH110X_WHITE);
  _display.fillRect(114, 44, 4, 8, SH110X_WHITE); // Battery tip
  
  // Fill width based on percentage (max fill area is 96 pixels wide)
  uint8_t fillWidth = (batPct * 96) / 100;
  _display.fillRect(16, 42, fillWidth, 12, SH110X_WHITE);

  _display.display();
}
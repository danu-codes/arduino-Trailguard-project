#include "DisplayManager.h"
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

DisplayManager::DisplayManager()
  : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
    _currentScreen(SCREEN_GPS),
    _inMenu(false),
    _menuIndex(0) {}

bool DisplayManager::begin(uint8_t i2cAddress) {
  if (!_display.begin(SSD1306_SWITCHCAPVCC, i2cAddress)) {
    return false;
  }
  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Set default foreground & background
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
  _currentScreen = static_cast<ScreenState>((_currentScreen + 1) % 5);
}

void DisplayManager::previousScreen() {
  _currentScreen = static_cast<ScreenState>((_currentScreen + 4) % 5);
}

// --- Menu Helpers ---
void DisplayManager::toggleMenu() {
  _inMenu = !_inMenu;
}

bool DisplayManager::isMenuOpen() const {
  return _inMenu;
}

void DisplayManager::closeMenu() {
  _inMenu = false;
}

void DisplayManager::nextMenuOption() {
  _menuIndex = (_menuIndex + 1) % MENU_OPTION_COUNT;
}

void DisplayManager::previousMenuOption() {
  _menuIndex = (_menuIndex + MENU_OPTION_COUNT - 1) % MENU_OPTION_COUNT;
}

MenuOption DisplayManager::getSelectedMenuOption() const {
  return static_cast<MenuOption>(_menuIndex);
}

void DisplayManager::drawHeader(const char* title, const String& temp, const String& hum) {
  // Always reset default text colors at the start of header rendering
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); 
  _display.setTextSize(1);
  _display.setCursor(0, 0);
  _display.print(title);

  _display.setCursor(85, 0);
  _display.print(temp);
  _display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
}

void DisplayManager::renderMenu() {
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  _display.setCursor(35, 0);
  _display.print("= MENU =");
  _display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

  const char* options[] = {
    "1. GPS Coordinates",
    "2. MPU Sensor Data",
    "3. Temp & Humidity",
    "4. System Info"
  };

  for (uint8_t i = 0; i < MENU_OPTION_COUNT; i++) {
    uint8_t y = 14 + (i * 12);
    if (i == _menuIndex) {
      _display.fillRect(0, y - 1, 128, 11, SSD1306_WHITE);
      _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Inverted text for highlighted row
    } else {
      _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Normal white text
    }
    _display.setCursor(4, y);
    _display.print(options[i]);
  }

  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  _display.display();
}

void DisplayManager::renderTempScreen(const String& temp, const String& hum) {
  _display.clearDisplay();
  drawHeader("ENVIRONMENT", temp, hum);

  _display.setCursor(0, 20);
  _display.setTextSize(1);
  _display.print("Temperature : ");
  _display.println(temp);
  _display.println();
  _display.print("Humidity    : ");
  _display.println(hum);
  _display.display();
}

void DisplayManager::drawOffCourseBanner() {
  if ((millis() / 500) % 2 == 0) {
    _display.fillRect(0, 12, 128, 12, SSD1306_WHITE);
    _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setCursor(18, 14);
    _display.print("! OFF COURSE !");
    _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  }
}

void DisplayManager::drawArrow(int16_t cx, int16_t cy, float angleDeg, int16_t radius) {
  float rad = (angleDeg - 90.0f) * M_PI / 180.0f;

  int16_t xTip = cx + cos(rad) * radius;
  int16_t yTip = cy + sin(rad) * radius;

  float wingAngle1 = rad + (135.0f * M_PI / 180.0f);
  float wingAngle2 = rad - (135.0f * M_PI / 180.0f);

  int16_t xWing1 = cx + cos(wingAngle1) * (radius * 0.6f);
  int16_t yWing1 = cy + sin(wingAngle1) * (radius * 0.6f);

  int16_t xWing2 = cx + cos(wingAngle2) * (radius * 0.6f);
  int16_t yWing2 = cy + sin(wingAngle2) * (radius * 0.6f);

  _display.drawCircle(cx, cy, radius + 2, SSD1306_WHITE);
  _display.drawLine(cx, cy, xTip, yTip, SSD1306_WHITE);
  _display.fillTriangle(xTip, yTip, xWing1, yWing1, xWing2, yWing2, SSD1306_WHITE);
}

void DisplayManager::renderBootScreen() {
  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
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
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  _display.setTextSize(1);
  _display.setCursor(0, 10);
  _display.println(title);
  _display.drawFastHLine(0, 22, 128, SSD1306_WHITE);
  _display.setCursor(0, 32);
  _display.println(msg);
  _display.display();
  if (delayMs > 0) {
    delay(delayMs);
  }
}

void DisplayManager::renderGPS(const GPSData& gps, const String& temp, const String& hum) {
  _display.clearDisplay();
  drawHeader("GPS MONITOR", temp, hum);

  _display.setCursor(0, 15);
  if (gps.fixValid) {
    _display.print("Lat: ");
    _display.println(gps.latitude, 5);
    _display.print("Lng: ");
    _display.println(gps.longitude, 5);
    _display.print("Alt: ");
    _display.print(gps.altitudeMeters, 1);
    _display.println("m");
    _display.print("Sats: ");
    _display.print(gps.satellites);
    _display.print(" Speed: ");
    _display.print(gps.speedKmh, 1);
    _display.println("km/h");
  } else {
    _display.println("\n  Searching Sats...");
    _display.print("  Sats Visible: ");
    _display.println(gps.satellites);
  }
  _display.display();
}

void DisplayManager::renderIMU(const MotionData& imu, const String& temp, const String& hum) {
  _display.clearDisplay();
  drawHeader("COMPASS / IMU", temp, hum);

  _display.setCursor(0, 16);
  _display.print("Heading: ");
  _display.print(imu.heading, 1);
  _display.println(" deg");
  _display.print("Pitch:   ");
  _display.print(imu.pitch, 1);
  _display.println(" deg");
  _display.print("Roll:    ");
  _display.print(imu.roll, 1);
  _display.println(" deg");
  _display.print("Steps:   ");
  _display.println(imu.stepCount);
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

void DisplayManager::renderReturnNav(double distance, double bearing, double heading,
                                     uint16_t currentIdx, uint16_t totalIdx,
                                     const String& temp, const String& hum,
                                     bool isOffCourse) {
  _display.clearDisplay();
  drawHeader("BACKTRACK NAV", temp, hum);

  if (isOffCourse) {
    drawOffCourseBanner();
  }

  float relativeAngle = bearing - heading;
  drawArrow(105, 42, relativeAngle, 14);

  uint8_t startY = isOffCourse ? 26 : 16;

  _display.setCursor(0, startY);
  _display.print("Dist: ");
  _display.print(distance, 1);
  _display.println("m");
  _display.setCursor(0, startY + 12);
  _display.print("Brg:  ");
  _display.print(bearing, 0);
  _display.println(" deg");
  _display.setCursor(0, startY + 24);
  _display.print("Pt:   ");
  _display.print(currentIdx);
  _display.print("/");
  _display.println(totalIdx);

  _display.display();
}
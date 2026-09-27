#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

// --- I2C Pins ---
#define I2C_SDA_PIN      8
#define I2C_SCL_PIN      9

// --- GPS UART Pins ---
#define GPS_RX_PIN       16
#define GPS_TX_PIN       17

// --- Battery ADC Sensing Pin ---
#define BATTERY_ADC_PIN  5

// --- Sensor & Peripheral Pins ---
#define BUZZER_PIN       11
#define DHT_PIN          4

// --- Status LED Pins ---
#define LED_GPS_PIN      6    // Green Status LED
#define LED_BLE_PIN      7    // Blue Status LED
#define LED_BAT_PIN      39   // Battery Status LED

// --- Keypad Button Pins (6 Physical Navigation/Action Buttons) ---
#define BTN_UP_PIN       1    
#define BTN_DOWN_PIN     2    
#define BTN_OK_PIN       14   
#define BTN_TRIP_PIN     15   
#define BTN_RETURN_PIN   3    
#define BTN_MENU_PIN     21   
#define BTN_BACK_PIN     10   

#endif // PIN_DEFINITIONS_H
#ifndef TEMP_MANAGER_H
#define TEMP_MANAGER_H

#include <Arduino.h>
#include <DHT.h>

class TempManager {
private:
    uint8_t _pin;
    uint8_t _type;
    DHT _dht;
    
    float _temperature;
    float _humidity;
    
    unsigned long _lastReadTime;
    const unsigned long READ_INTERVAL = 2000;

public:
    TempManager(uint8_t pin, uint8_t type = DHT22);

    void begin();
    void update();

    float getTemperature();
    float getHumidity();
    
    String getTempString();
    String getHumidityString();
    bool isDataValid();
};

#endif
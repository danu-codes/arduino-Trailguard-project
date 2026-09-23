#include "TempManager.h"

TempManager::TempManager(uint8_t pin, uint8_t type) 
    : _pin(pin), _type(type), _dht(pin, type) {
    _temperature = 0.0;
    _humidity = 0.0;
    _lastReadTime = 0;
}

void TempManager::begin() {
    _dht.begin();
    _lastReadTime = millis();
}

void TempManager::update() {
    if (millis() - _lastReadTime >= READ_INTERVAL) {
        _lastReadTime = millis();

        float t = _dht.readTemperature();
        float h = _dht.readHumidity();

        if (!isnan(t) && !isnan(h)) {
            _temperature = t;
            _humidity = h;
        }
    }
}

float TempManager::getTemperature() {
    return _temperature;
}

float TempManager::getHumidity() {
    return _humidity;
}

String TempManager::getTempString() {
    if (isnan(_temperature) || _temperature == 0.0) return "--.- C";
    return String(_temperature, 1) + " C";
}

String TempManager::getHumidityString() {
    if (isnan(_humidity) || _humidity == 0.0) return "-- %";
    return String((int)_humidity) + " %";
}

bool TempManager::isDataValid() {
    return (!isnan(_temperature) && !isnan(_humidity) && _temperature != 0.0);
}
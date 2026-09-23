#include "RouteManager.h"
#include <math.h>

RouteManager::RouteManager() 
  : _pointCount(0), _currentTargetIndex(0), _recording(false), _returnMode(false) {}

void RouteManager::startRecording() {
  _pointCount = 0;
  _recording = true;
  _returnMode = false;
}

void RouteManager::stopRecording() {
  _recording = false;
}

bool RouteManager::isRecording() const {
  return _recording;
}

void RouteManager::startReturnMode() {
  if (_pointCount > 0) {
    _recording = false;
    _returnMode = true;
    _currentTargetIndex = _pointCount - 1;
  }
}

void RouteManager::stopReturnMode() {
  _returnMode = false;
}

bool RouteManager::isReturnMode() const {
  return _returnMode;
}

bool RouteManager::addBreadcrumb(double lat, double lng) {
  if (!_recording || _pointCount >= MAX_POINTS) return false;

  if (_pointCount > 0) {
    double dist = calculateDistance(_breadcrumbs[_pointCount - 1].lat, _breadcrumbs[_pointCount - 1].lng, lat, lng);
    if (dist < 5.0) return false;
  }

  _breadcrumbs[_pointCount].lat = lat;
  _breadcrumbs[_pointCount].lng = lng;
  _pointCount++;
  return true;
}

void RouteManager::updateNavigation(double currentLat, double currentLng) {
  if (!_returnMode || _pointCount == 0) return;

  double dist = calculateDistance(currentLat, currentLng, 
                                  _breadcrumbs[_currentTargetIndex].lat, 
                                  _breadcrumbs[_currentTargetIndex].lng);

  if (dist < 5.0 && _currentTargetIndex > 0) {
    _currentTargetIndex--;
  }
}

bool RouteManager::isOffCourse(double currentLat, double currentLng) const {
  if (!_returnMode || _pointCount == 0) return false;

  double distToTarget = calculateDistance(
    currentLat, currentLng,
    _breadcrumbs[_currentTargetIndex].lat,
    _breadcrumbs[_currentTargetIndex].lng
  );

  return (distToTarget > 15.0);
}

double RouteManager::getDistanceToNext(double currentLat, double currentLng) const {
  if (_pointCount == 0) return 0.0;
  return calculateDistance(currentLat, currentLng, 
                           _breadcrumbs[_currentTargetIndex].lat, 
                           _breadcrumbs[_currentTargetIndex].lng);
}

double RouteManager::getBearingToNext(double currentLat, double currentLng) const {
  if (_pointCount == 0) return 0.0;
  return calculateBearing(currentLat, currentLng, 
                          _breadcrumbs[_currentTargetIndex].lat, 
                          _breadcrumbs[_currentTargetIndex].lng);
}

double RouteManager::calculateDistance(double lat1, double lon1, double lat2, double lon2) const {
  double R = 6371000;
  double dLat = (lat2 - lat1) * M_PI / 180.0;
  double dLon = (lon2 - lon1) * M_PI / 180.0;
  double a = sin(dLat / 2) * sin(dLat / 2) +
             cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
             sin(dLon / 2) * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return R * c;
}

double RouteManager::calculateBearing(double lat1, double lon1, double lat2, double lon2) const {
  double dLon = (lon2 - lon1) * M_PI / 180.0;
  double y = sin(dLon) * cos(lat2 * M_PI / 180.0);
  double x = cos(lat1 * M_PI / 180.0) * sin(lat2 * M_PI / 180.0) -
             sin(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * cos(dLon);
  double brng = atan2(y, x) * 180.0 / M_PI;
  return fmod((brng + 360.0), 360.0);
}

uint16_t RouteManager::getPointCount() const { return _pointCount; }
uint16_t RouteManager::getCurrentTargetIndex() const { return _currentTargetIndex; }
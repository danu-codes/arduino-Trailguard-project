#include "RouteManager.h"
#include <math.h>

RouteManager::RouteManager()
  : _pointCount(0), _currentTargetIndex(0), _recording(false), _returnMode(false), _totalDistanceMeters(0.0), _returnDistanceMeters(0.0), _hasLastReturnPos(false) {}

void RouteManager::startRecording() {
  _pointCount = 0;
  _recording = true;
  _returnMode = false;
  _totalDistanceMeters = 0.0;  // Reset live distance at trip start
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
    _returnDistanceMeters = 0.0;
    _hasLastReturnPos = false;  // Reset tracking for the return journey
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

  // Check distance only if we ALREADY have logged points
  if (_pointCount > 0) {
    double dist = calculateDistance(_breadcrumbs[_pointCount - 1].lat, _breadcrumbs[_pointCount - 1].lng, lat, lng);

    // Ignore movements smaller than 10.0 meters to filter out static GPS jitter while standing still
    if (dist < 10.0) return false;

    // Accumulate valid distance moved
    _totalDistanceMeters += dist;
  }

  // Record point safely
  _breadcrumbs[_pointCount].lat = lat;
  _breadcrumbs[_pointCount].lng = lng;
  _pointCount++;
  return true;
}

void RouteManager::updateNavigation(double currentLat, double currentLng) {
  if (!_returnMode || _pointCount == 0) return;

  // Track distance traveled during return mode
  if (!_hasLastReturnPos) {
    _lastReturnLat = currentLat;
    _lastReturnLng = currentLng;
    _hasLastReturnPos = true;
  } else {
    double moveDist = calculateDistance(_lastReturnLat, _lastReturnLng, currentLat, currentLng);
    if (moveDist >= 5.0) {  // Filter out GPS jitter under 5 meters
      _returnDistanceMeters += moveDist;
      _lastReturnLat = currentLat;
      _lastReturnLng = currentLng;
    }
  }

  double dist = calculateDistance(currentLat, currentLng,
                                  _breadcrumbs[_currentTargetIndex].lat,
                                  _breadcrumbs[_currentTargetIndex].lng);

  // Switch to the next target waypoint when within 6.0 meters of current waypoint
  if (dist < 6.0) {
    if (_currentTargetIndex > 0) {
      _currentTargetIndex--;
    } else {
      // Reached starting origin point! Return complete.
      _returnMode = false;
    }
  }
}

bool RouteManager::isOffCourse(double currentLat, double currentLng) const {
  if (!_returnMode || _pointCount == 0) return false;

  double distToTarget = calculateDistance(
    currentLat, currentLng,
    _breadcrumbs[_currentTargetIndex].lat,
    _breadcrumbs[_currentTargetIndex].lng);

  if (_currentTargetIndex < _pointCount - 1) {
    double distToPrev = calculateDistance(
      currentLat, currentLng,
      _breadcrumbs[_currentTargetIndex + 1].lat,
      _breadcrumbs[_currentTargetIndex + 1].lng);

    double segmentDist = calculateDistance(
      _breadcrumbs[_currentTargetIndex + 1].lat, _breadcrumbs[_currentTargetIndex + 1].lng,
      _breadcrumbs[_currentTargetIndex].lat, _breadcrumbs[_currentTargetIndex].lng);

    if ((distToTarget + distToPrev) > (segmentDist + 20.0)) {
      return true;
    }
    return false;
  }

  return (distToTarget > 30.0);
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

double RouteManager::getTotalDistance() const {
  return _totalDistanceMeters;
}

double RouteManager::calculateDistance(double lat1, double lon1, double lat2, double lon2) const {
  double R = 6371000.0;  // Earth's radius in meters
  double dLat = (lat2 - lat1) * M_PI / 180.0;
  double dLon = (lon2 - lon1) * M_PI / 180.0;
  double a = sin(dLat / 2.0) * sin(dLat / 2.0) + cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dLon / 2.0) * sin(dLon / 2.0);
  double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
  return R * c;
}

double RouteManager::calculateBearing(double lat1, double lon1, double lat2, double lon2) const {
  double dLon = (lon2 - lon1) * M_PI / 180.0;
  double y = sin(dLon) * cos(lat2 * M_PI / 180.0);
  double x = cos(lat1 * M_PI / 180.0) * sin(lat2 * M_PI / 180.0) - sin(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * cos(dLon);
  double brng = atan2(y, x) * 180.0 / M_PI;
  return fmod((brng + 360.0), 360.0);
}

uint16_t RouteManager::getPointCount() const {
  return _pointCount;
}
uint16_t RouteManager::getCurrentTargetIndex() const {
  return _currentTargetIndex;
}
double RouteManager::getReturnDistance() const {
  return _returnDistanceMeters;
}
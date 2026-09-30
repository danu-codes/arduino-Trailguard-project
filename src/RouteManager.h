#ifndef ROUTE_MANAGER_H
#define ROUTE_MANAGER_H

#include <Arduino.h>
#include "GPSManager.h"

struct Waypoint {
  double lat;
  double lng;
};

class RouteManager {
public:
  RouteManager();

  void startRecording();
  void stopRecording();
  bool isRecording() const;

  void startReturnMode();
  void stopReturnMode();
  bool isReturnMode() const;
  double getReturnDistance() const;

  bool addBreadcrumb(double lat, double lng);
  bool isOffCourse(double currentLat, double currentLng) const;

  double getDistanceToNext(double currentLat, double currentLng) const;
  double getBearingToNext(double currentLat, double currentLng) const;
  double getTotalDistance() const;  // <--- Returns live accumulated trip distance

  uint16_t getPointCount() const;
  uint16_t getCurrentTargetIndex() const;

  void updateNavigation(double currentLat, double currentLng);

private:
  static const uint16_t MAX_POINTS = 300;
  Waypoint _breadcrumbs[MAX_POINTS];
  uint16_t _pointCount;
  uint16_t _currentTargetIndex;

  bool _recording;
  bool _returnMode;
  double _totalDistanceMeters;  // <--- Tracks live distance
  double _returnDistanceMeters;
  double _lastReturnLat;
  double _lastReturnLng;
  bool _hasLastReturnPos;

  double calculateDistance(double lat1, double lon1, double lat2, double lon2) const;
  double calculateBearing(double lat1, double lon1, double lat2, double lon2) const;
};

#endif  // ROUTE_MANAGER_H
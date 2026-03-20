#ifndef _MAGNETOMETER_H
#define _MAGNETOMETER_H
#include <Wire.h>
#include <LIS3MDL.h>

#define MAX_AXIS 3
#define MAG_THRESHOLD 2.4

class Magnetometer_c {
  public:
    LIS3MDL mag;
    float readings[MAX_AXIS];
    
    float raw_magnitude;
    float calibrated;
    float min_val;
    float max_val;
    bool calibrating;

    // Constructor
    Magnetometer_c() {
      calibrating = false;
      min_val = 0;
      max_val = 0;
      calibrated = 0;
      raw_magnitude = 0;
    }

    // Call this function within your setup() function
    bool initialise() {
      Wire.begin();
      
      if (!mag.init()) {
        return false;
      }
      mag.enableDefault();
      return true;
    }

    // Function to update readings array with latest values
    void getReadings() {
      mag.read();
      readings[0] = mag.m.x;
      readings[1] = mag.m.y;
      readings[2] = mag.m.z;
      
      raw_magnitude = sqrt(
        // readings[0] * readings[0] + 
        // readings[1] * readings[1] +
        readings[2] * readings[2]
      );
    }

    void calibrationSetup() {
      getReadings();
      min_val = raw_magnitude;
      max_val = raw_magnitude;
      calibrating = true;
    }

    void calibrationUpdate() {
      if (!calibrating) return;
      
      getReadings();
      if (raw_magnitude < min_val) min_val = raw_magnitude;
      if (raw_magnitude > max_val) max_val = raw_magnitude;
    }

    void calibrationFinish() {
      calibrating = false;
    }

    void calcCalibrated() {
      getReadings();
      float range = max_val - min_val;
      if (range == 0) {
        calibrated = 0;
        return;
      }
      calibrated = ((raw_magnitude - min_val) / range);
    }

    bool detectMagnet() {
      return calibrated >= MAG_THRESHOLD;
    }
};

#endif
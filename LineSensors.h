#ifndef _LINESENSORS_H
#define _LINESENSORS_H
#define NUM_SENSORS 3
#define NUM_BUMP_SENSORS 2
#define EMIT_PIN 11 // infra-red LEDs PIN
#define BLACK_THRESHOLD 0.9

const int sensor_pins[ NUM_SENSORS ] = { A0, A2, A3 };
const int digital_pins[ NUM_BUMP_SENSORS ] = { 4, 5 };

class LineSensors_c {
  
  public:
    int digital_readings[ NUM_BUMP_SENSORS ];
    float readings[ NUM_SENSORS ];
    float minimum[ NUM_SENSORS ];
    float maximum[ NUM_SENSORS ];
    float scaling[ NUM_SENSORS ];
    float calibrated[ NUM_SENSORS ];

    LineSensors_c() {
    }

    void initialiseForADC() {
        pinMode( EMIT_PIN, INPUT );
        // Configure the line sensor pins
        // DN1, DN2, DN3, DN4, DN5.
        for ( int sensor = 0; sensor < NUM_SENSORS; sensor++ ) {
        pinMode( sensor_pins[sensor], INPUT_PULLUP );
        }
    }

    void irOn() {
        pinMode(EMIT_PIN, OUTPUT);
        digitalWrite(EMIT_PIN, LOW);  // LOW = emitter on (active low)
    }

    void irOff() {
        pinMode(EMIT_PIN, INPUT);     // High-Z = emitter off
    }

    void readSensorsADC() {
        for( int sensor = 0; sensor < NUM_SENSORS; sensor++ ) {
        readings[sensor] = analogRead( sensor_pins[sensor] );
        }
    }

    void calibrationSetup() {
        // Initialize min/max arrays with opposite extremes
        for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
            minimum[sensor] = 1023.0;
            maximum[sensor] = 0.0;
        }
    }

    void calibrationUpdate() {
        // Read sensors and update min/max if needed
        readSensorsADC();
        
        for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
            if (readings[sensor] < minimum[sensor]) {
                minimum[sensor] = readings[sensor];
            }
            if (readings[sensor] > maximum[sensor]) {
                maximum[sensor] = readings[sensor];
            }
        }
    }

    void calibrationFinish() {
        // Calculate scaling factors (range) for each sensor
        for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
            scaling[sensor] = maximum[sensor] - minimum[sensor];

            if (scaling[sensor] < 1.0) {
                scaling[sensor] = 1.0;
            }
        }
    }

    void calcCalibratedADC() {
        readSensorsADC();
        for( int sensor = 0; sensor < NUM_SENSORS; sensor++ ) {
        calibrated[sensor] = (readings[sensor] - minimum[sensor]) / scaling[sensor];
        }
    }
    
    bool isOnBlack(int sensor_index) {
        if (sensor_index < 0 || sensor_index >= NUM_SENSORS) return false;
        return calibrated[sensor_index] >= BLACK_THRESHOLD;
    }

    void initialiseForDigital() {
        for (int sensor = 0; sensor < NUM_BUMP_SENSORS; sensor++) {
            pinMode(digital_pins[sensor], INPUT_PULLUP);
        }
    }

    void readSensorsDigital() {
        for (int sensor = 0; sensor < NUM_BUMP_SENSORS; sensor++) {
            digital_readings[sensor] = digitalRead(digital_pins[sensor]);
        }
    }
};

#endif
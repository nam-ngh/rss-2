#include "LineSensors.h"
#include "lcd.h"

LCD_c display(0,1,14,17,13,30);
LineSensors_c line_sensors;

#define BUZZER_PIN 6
#define BIT_PERIOD_US 5000 // 1ms per bit = 1kbps
#define THRESHOLD -1.5
#define MSG_LEN 20
#define TRIALS 10

int curr_idx = 0;
int curr_trial = 1;
int correct = 0;

void shortBeep(int duration) {
  analogWrite(BUZZER_PIN, 120);
  delay(duration);
  analogWrite(BUZZER_PIN, 0);
}

void sendByte(uint8_t value) {
    // Start bit (HIGH)
    line_sensors.irOn();
    delayMicroseconds(BIT_PERIOD_US);

    // 8 data bits, LSB first
    for (int i = 0; i < 8; i++) {
        if (value & (1 << i)) line_sensors.irOn();
        else line_sensors.irOff();
        delayMicroseconds(BIT_PERIOD_US);
    }

    // Stop bit (LOW = idle)
    line_sensors.irOff();
    delayMicroseconds(BIT_PERIOD_US);
}

void calibrateSensors() {
  line_sensors.calibrationSetup();
  unsigned long start = millis();
  while (millis() - start < 3000) {
    line_sensors.calibrationUpdate();
    delay(10);
  }
  line_sensors.calibrationFinish();
}

auto printSensors = [&]() {
    for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.print("s");
        Serial.print(i);
        Serial.print(":");
        Serial.print(line_sensors.calibrated[i]);
        Serial.print(" ");
    }
    Serial.println();
};

auto printSensorsRaw = [&]() {
    for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.print("s");
        Serial.print(i);
        Serial.print(":");
        Serial.print(line_sensors.readings[i]);
        Serial.print(" ");
    }
    Serial.println();
};

auto printDigitalSensors = [&]() {
    for (int i = 0; i < NUM_BUMP_SENSORS; i++) {
        Serial.print("b");
        Serial.print(i);
        Serial.print(":");
        Serial.print(line_sensors.digital_readings[i]);
        Serial.print(" ");
    }
    Serial.println();
};

float avgSensors() {
    float sum = 0;
    for (int i = 0; i < NUM_SENSORS; i++) {
        sum += line_sensors.calibrated[i];
    }
    return sum / NUM_SENSORS;
}

void setup() {
    Serial.begin(9600);
    while (!Serial);
    pinMode(BUZZER_PIN, OUTPUT);
    line_sensors.initialiseForADC();
    display.init();
    display.clear();
    delay(4000);

    shortBeep(200);
    calibrateSensors();
    Serial.println("IR emitter ready.");
    unsigned long t1 = micros();
    line_sensors.calcCalibratedADC();
    unsigned long t2 = micros();
    Serial.print("calcCalibratedADC took: ");
    Serial.println(t2 - t1);
}

void loop() {
    if (curr_trial > TRIALS) {
        Serial.print("Trials completed. Accuracy: ");
        Serial.print((float)correct / (MSG_LEN * TRIALS) * 100.0);
        Serial.println("%");
        while (true);
    }

    line_sensors.calcCalibratedADC();

    // if start bit detected enter loop
    if (avgSensors() < THRESHOLD) {
        unsigned long startTime = micros();  // anchor point
        Serial.println("Start of message " + String(curr_idx));
        // printSensors();
      
      
        uint8_t received = 0;
        for (int i = 0; i < 8; i++) {
            unsigned long targetTime = startTime + (unsigned long)(BIT_PERIOD_US * i) + (BIT_PERIOD_US * 3/2);
            while (micros() < targetTime);  // busy-wait to exact sample point
            
            line_sensors.calcCalibratedADC();
            // printSensors();
            if (avgSensors() < THRESHOLD) {
                received |= (1 << i);
            }
        }
    
        if (received == curr_idx) {
            correct++;
        }
        Serial.print("Trial: ");
        Serial.print(curr_trial);
        Serial.print(" : Values: ");
        Serial.print(curr_idx);
        Serial.print("--");
        Serial.println(received);
        
        curr_idx++;

        if (curr_idx >= MSG_LEN) {
            Serial.print("Trial ");
            Serial.print(curr_trial);
            Serial.println(" completed.");
            curr_idx = 0;
            curr_trial++;
        }
    }
}
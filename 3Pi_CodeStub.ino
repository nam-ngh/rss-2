#include "LineSensors.h"
#include "lcd.h"

LCD_c display(0,1,14,17,13,30);
LineSensors_c line_sensors;

#define BUZZER_PIN 6
#define BIT_PERIOD_US 104 // 1ms per bit = 1kbps
#define THRESHOLD -1.5

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
    for (int i = 1; i < 4; i++) {
        Serial.print("s");
        Serial.print(i);
        Serial.print(":");
        Serial.print(line_sensors.calibrated[i]);
        Serial.print(" ");
    }
    Serial.println();
};

float avgSensors() {
    float sum = 0;
    for (int i = 1; i < 4; i++) {
        sum += line_sensors.calibrated[i];
    }
    return sum / 5.0f;
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
}

//void loop() {
//    sendByte(0b00000001);
//    line_sensors.irOff();
//    delayMicroseconds(BIT_PERIOD_US * 2);  // 2 idle periods between frames
//}

void loop() {
    // Phase 1 - waiting for start bit
    line_sensors.calcCalibratedADC();
//    Serial.print("sensor1:");
//    Serial.print(line_sensors.calibrated[1]);
//    Serial.print(",");
//    Serial.print("sensor2:");
//    Serial.print(line_sensors.calibrated[2]);
//    Serial.print(",");
//    Serial.print("sensor3:");
//    Serial.print(line_sensors.calibrated[3]);
//    Serial.print(",");
//    Serial.print("sensor4:");
//    Serial.println(line_sensors.calibrated[4]);
//    delay(100);

    if (avgSensors() < THRESHOLD) {
      Serial.println("Start bit detected");
      printSensors();
      
      unsigned long startTime = micros();  // anchor point
      
      uint8_t received = 0;
      for (int i = 0; i < 8; i++) {
          // Wait until the MIDDLE of bit i, relative to start bit beginning
          // Bit i starts at (i+1) * BIT_PERIOD_US from startTime
          // Middle of bit i is at (i+1.5) * BIT_PERIOD_US
          unsigned long targetTime = startTime + (unsigned long)(BIT_PERIOD_US * i) + (BIT_PERIOD_US * 3 / 2);
          while (micros() < targetTime);  // busy-wait to exact sample point
          
          line_sensors.calcCalibratedADC();
          printSensors();
          if (avgSensors() < THRESHOLD) {
              received |= (1 << i);
          }
      }
  
      // Stop bit sample
      unsigned long stopTarget = startTime + (unsigned long)(9.5f * BIT_PERIOD_US);
      while (micros() < stopTarget);
      line_sensors.calcCalibratedADC();
      printSensors();
  
      if (avgSensors() >= THRESHOLD) {
          Serial.println(received);
      }
    }
}
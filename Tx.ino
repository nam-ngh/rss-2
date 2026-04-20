// comment out the entire document when not in use

#include "LineSensors.h"

LineSensors_c line_sensors;

#define BUZZER_PIN 6
#define BIT_PERIOD_US 5000 // 1ms per bit = 1kbps
#define THRESHOLD -1.5
#define FRAME_INTERVAL_MS 550

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

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  line_sensors.initialiseForADC();
  delay(4000);
  
  shortBeep(200);
  calibrateSensors();
  Serial.println("IR emitter ready.");
}

void loop() {
    unsigned long trialStart = millis();
    for (uint8_t i = 0; i < 20; i++) {
        unsigned long frameStart = trialStart + (unsigned long)i * FRAME_INTERVAL_MS;
        while (millis() < frameStart);  // wait until exact scheduled time to send
        sendByte(i);
    }
    delay(2000);
}

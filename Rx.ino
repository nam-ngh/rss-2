#include "LineSensors.h"
#include "lcd.h"

LCD_c display(0,1,14,17,13,30);
LineSensors_c line_sensors;

#define BUZZER_PIN 6
#define BIT_PERIOD_US 5000 // microseconds, should be divisible by 100 for frame (ms) timing conversion
#define THRESHOLD -1.5
#define MSG_LEN 20
#define TRIALS 3
#define FRAME_INTERVAL_MS 550

int received_idx = 0;
int curr_trial = 1;
int correct = 0;
int total_bit_errors = 0;
unsigned long trialStartTime = 0;
bool trialActive = false;

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
        while (true);
    }

    line_sensors.calcCalibratedADC();

    // if start bit detected enter loop
    if (avgSensors() < THRESHOLD) {
        unsigned long startTime = micros();  // anchor point
        if (received_idx == 0) {
            trialStartTime = millis();
            trialActive = true;
        }
        uint8_t expected_idx = (millis() - trialStartTime) / FRAME_INTERVAL_MS;
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
        // // Wait until full frame to start processing result
        // unsigned long lockoutEnd = startTime + (unsigned long)(BIT_PERIOD_US * 10);
        // while (micros() < lockoutEnd);
        
        // received value validation, print will take time in millis, 
        // requires Tx to have delay between frames to avoid overlap
        // minimum delay of 200ms would be safe
        uint8_t diff = received ^ expected_idx;
        uint8_t bit_err_count = __builtin_popcount(diff);
        Serial.print("Values: ");
        Serial.print(expected_idx);
        Serial.print("--");

        if (received == expected_idx) {
            correct++;
            Serial.println(received);
        } else {
            Serial.print(received);
            Serial.print(". Bits dropped: ");
            Serial.println(bit_err_count);
        }
        
        received_idx++;
        total_bit_errors += bit_err_count;
    }
    uint8_t expected_idx = (millis() - trialStartTime) / FRAME_INTERVAL_MS;
    if (trialActive && expected_idx >= MSG_LEN) {
        Serial.print("Trial ");
        Serial.print(curr_trial);
        Serial.print(" completed. Correct: ");
        Serial.print((int)correct);
        Serial.print(". Faults: ");
        Serial.print((int)(received_idx - correct));
        Serial.print(". BER: ");
        Serial.print((float)(total_bit_errors) / ((received_idx) * 8) * 100.0);
        Serial.print("%. PER: ");
        Serial.print((float)(received_idx - correct) / (received_idx) * 100.0);
        Serial.print("%. Accuracy: ");
        Serial.print((float)correct / (received_idx) * 100.0);
        Serial.print("%. Missed frames: ");
        Serial.println((int)(expected_idx - received_idx));

        received_idx = 0;
        correct = 0;
        total_bit_errors = 0;
        curr_trial++;
        trialActive = false;
    }
}
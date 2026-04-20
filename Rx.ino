#include "LineSensors.h"
#include "lcd.h"

LCD_c display(0,1,14,17,13,30);
LineSensors_c line_sensors;

#define BUZZER_PIN 6
#define BIT_PERIOD_US 5000 // microseconds, should be divisible by 100 for frame (ms) timing conversion
#define THRESHOLD -1.5
#define TRIALS 10
#define TRIAL_TIME_MS (550 * 20)
#define MAX_SIZE 20

bool decoding = false;
bool trialActive = false;
unsigned long trialStartTime = 0;
uint8_t curr_trial = 0;
uint8_t results[MAX_SIZE];
int arrayIdx = 0;


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

    // if start bit detected enter decoding loop
    if (avgSensors() < THRESHOLD && !decoding) {
        if (!trialActive) {
            trialActive = true;
            trialStartTime = millis();
        }
        decoding = true;
        unsigned long startTime = micros();
        
        // main byte decoding loop
        uint8_t received = 0;
        for (int i = 0; i < 8; i++) {
            unsigned long targetTime = startTime + (unsigned long)(BIT_PERIOD_US * i) + (BIT_PERIOD_US * 3/2);
            while (micros() < targetTime);  // busy-wait to exact sample point
            
            line_sensors.calcCalibratedADC();
            if (avgSensors() < THRESHOLD) {
                received |= (1 << i);
            }
        }
        // lockout to prevent unexpected triggers mid-frame
        unsigned long lockoutEnd = startTime + (unsigned long)(BIT_PERIOD_US * 10) - 500;
        while (micros() < lockoutEnd);
        decoding = false;
        if (arrayIdx < MAX_SIZE) {
            results[arrayIdx++] = received;
            Serial.println(received);
        }
    }
    if (arrayIdx >= MAX_SIZE || (trialActive && millis() - trialStartTime > TRIAL_TIME_MS)) {
        Serial.print("Trial ");
        Serial.print(curr_trial);
        Serial.println(". Received: ");
        uint8_t total_bit_errors = 0;
        for (int i = 0; i < MAX_SIZE; i++) {
            Serial.print(results[i]);
            Serial.print(" ");
            total_bit_errors += __builtin_popcount(results[i] ^ i);
        }
        Serial.println();
        if (arrayIdx == MAX_SIZE) {
            Serial.print("Indexes matched, total bit errors: ");
            Serial.println(total_bit_errors);
        }
        else {
            Serial.print("Received ");
            Serial.print(arrayIdx);
            Serial.println(" out of 20 messages.");
        }
        arrayIdx = 0;
        memset(results, 0, sizeof(results));
        trialActive = false;
        curr_trial++;
    }
}
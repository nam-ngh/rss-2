#ifndef _MOTORS_H
#define _MOTORS_H

#define L_PWM 10
#define L_DIR 16
#define R_PWM 9
#define R_DIR 15
#define MAX_PWM 60.0
#define MIN_PWM 20.0

// Class to operate the motors.
class Motors_c {
  public:
    // Constructor, must exist.
    Motors_c() {}
    void initialise() {
      pinMode(L_PWM, OUTPUT);
      pinMode(L_DIR, OUTPUT);
      pinMode(R_PWM, OUTPUT);
      pinMode(R_DIR, OUTPUT);

      // Set the direction pins to a default value
      digitalWrite( L_DIR, LOW );
      digitalWrite( R_DIR, LOW );
      // Set the PWM pins to a default value of 0 (off)
      analogWrite( L_PWM, 0 );
      analogWrite( R_PWM, 0 );
    }

    void setPWM( float left_pwr, float right_pwr ) {
      if ( left_pwr < 0 ) {
        digitalWrite( L_DIR, HIGH );
        left_pwr = -left_pwr;
      } else {
        digitalWrite( L_DIR, LOW );
      }

      if ( right_pwr < 0 ) {
        digitalWrite( R_DIR, HIGH );
        right_pwr = -right_pwr;
      } else {
        digitalWrite( R_DIR, LOW );
      }
      analogWrite( L_PWM, left_pwr );
      analogWrite( R_PWM, right_pwr );
      return;
    }
};

#endif
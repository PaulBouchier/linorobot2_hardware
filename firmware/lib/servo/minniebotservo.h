#ifndef MINNIEBOTSERVO_H
#define MINNIEBOTSERVO_H

#include "ESP32Servo.h"

void initServo();
void setServoUs(int value); // set servo pulse width in microseconds

#endif // MINNIEBOTSERVO_H
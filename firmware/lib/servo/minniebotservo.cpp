#include <Arduino.h>
#include "config.h"
#include "minniebotservo.h"

Servo minniebotServo;

void initServo()
{
    ESP32PWM::allocateTimer(3);
    minniebotServo.attach(SERVO_PIN);
    minniebotServo.writeMicroseconds(SERVO_DEFAULT_US); // set default position
}
void setServoUs(int value)
{
    minniebotServo.writeMicroseconds(value);
}
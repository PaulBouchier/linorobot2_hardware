#ifndef ROBOREMO_H
#define ROBOREMO_H

#include <Arduino.h>
#include "syslog.h"
#include <geometry_msgs/msg/twist.h>

// Check if Bluetooth is properly configured
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it.
#endif

class Roboremo {
public:
    Roboremo(geometry_msgs__msg__Twist & twist_msg, unsigned long & prev_cmd_time);
    void begin();
    void loop();

private:
    void exeCmd();
    void sendTeleop(float v, float rotSpeed);
    void sendAutoRun(int disable);

    char cmd[100];
    int cmdIndex;
    float maxSpeed;
    float maxRotSpeed;
    float axf, ayf, azf;
    int teleopTime;
    int lastTeleopTime;
    bool teleopActive;
    int disableAutoRun;
    int lastDisableAutoRun;
    int deadmanTime;
    int lastDeadmanTime;
    bool deadmanActive;
    int nextPingTime;
    geometry_msgs__msg__Twist & twist_msg;
    unsigned long & prev_cmd_time;
};

#endif

#ifndef ROBOREMO_H
#define ROBOREMO_H

#include <Arduino.h>
#include "BluetoothSerial.h"
#include "syslog.h"

class Roboremo {
public:
    Roboremo();
    void begin();
    void loop();

private:
    void exeCmd();
    void sendTeleop(float v, float rotSpeed);
    void sendAutoRun(int disable);

    BluetoothSerial SerialBT;
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
};

#endif

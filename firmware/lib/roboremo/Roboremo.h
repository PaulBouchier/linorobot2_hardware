#ifndef ROBOREMO_H
#define ROBOREMO_H

#include <Arduino.h>
#include "BluetoothSerial.h"
#include "syslog.h"

// Check if Bluetooth is properly configured
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it.
#endif

class Roboremo {
public:
    Roboremo();
    ~Roboremo();
    void begin();
    void loop();

private:
    void exeCmd();
    void sendTeleop(float v, float rotSpeed);
    void sendAutoRun(int disable);

    // Function declarations for Core 1 Bluetooth processing
    static void btTaskWorker(void * pvParameters);
    void btTaskLoop();

    // Task Handle for Bluetooth processing
    TaskHandle_t BTTaskHandle = NULL;
    // The BluetoothSerial object for handling Bluetooth communication
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

#include "Roboremo.h"
#include <string>
#include <cstdlib>

Roboremo::Roboremo() : 
    cmdIndex(0),
    maxSpeed(0.35),
    maxRotSpeed(0.3),
    axf(0.0), ayf(0.0), azf(0.0),
    teleopTime(0),
    lastTeleopTime(0),
    teleopActive(false),
    disableAutoRun(0),
    lastDisableAutoRun(0),
    deadmanTime(0),
    lastDeadmanTime(0),
    deadmanActive(false),
    nextPingTime(0)
{
    memset(cmd, 0, sizeof(cmd));
}

Roboremo::~Roboremo() {
    if (BTTaskHandle != NULL) {
        vTaskDelete(BTTaskHandle);
    }
    SerialBT.end();
}

void Roboremo::begin() {
    return; // Disable Bluetooth processing for now
    SerialBT.begin("Mowberry");  // Bluetooth device name

    xTaskCreatePinnedToCore(
        btTaskWorker,           /* Task function. */
        "BT_Serial_Task",       /* Name of task. */
        4096,                   /* Stack size in words. */
        this,                   /* Parameter of the task */
        1,                      /* Priority of the task (Low-to-medium) */
        &BTTaskHandle,          /* Task handle to keep track of created task */
        1                       /* Pin task to Core 1 (Leaves Core 0 for radio stacks) */
    );

    cmdIndex = 0;
    axf = 0.0;
    ayf = 0.0;
    azf = 0.0;
    teleopTime = millis();
    lastTeleopTime = 0;
    teleopActive = false;
    disableAutoRun = 0;
    lastDisableAutoRun = 0;
    deadmanTime = millis();
    lastDeadmanTime = 0;
    deadmanActive = false;
    nextPingTime = millis() + 500;
}

// Thread-isolated Bluetooth execution logic running on Core 1
void Roboremo::btTaskWorker (void * pvParameters) {
    Roboremo* roboremo = static_cast<Roboremo*>(pvParameters);
    roboremo->btTaskLoop();
    // Crucial: yield execution back to FreeRTOS scheduler to prevent crashes
    vTaskDelay(pdMS_TO_TICKS(5)); 
}

void Roboremo::btTaskLoop() {
    return; // Disable Bluetooth processing for now

    for(;;) {
        if (SerialBT.available()) {
            char incomingByte = SerialBT.read();
            //cmd[cmdIndex] = c;
            //if (cmdIndex < 99)
            //    cmdIndex++;
            //cmd[cmdIndex] = '\0';

            if (incomingByte == '\n') {
                syslog(LOG_INFO, "Received Bluetooth command: %s", cmd);
                // Process incoming Bluetooth Serial commands safely here
                // exeCmd();
                //cmdIndex = 0;
            }
        }
    }
}

void Roboremo::sendTeleop(float v, float rotSpeed) {
    syslog(LOG_INFO, "%s: Teleop v: %0.2f r: %0.2f", __FUNCTION__, v, rotSpeed);
}

void Roboremo::sendAutoRun(int disable) {
    syslog(LOG_INFO, "%s: Auto-run disable %d", __FUNCTION__, disable);
}

void Roboremo::exeCmd() {
    std::string rcvString = std::string(cmd);
    std::string cmdString = rcvString.substr(0, 2);
    float vel = 0.0;
    float rotSpeed = 0.0;
    float velFwd;
    float velBackwd;

    if (std::string("ax") == cmdString) {
        axf = atof(&cmd[3]);
    } else if (std::string("ay") == cmdString) {
        ayf = atof(&cmd[3]);
    } else if (std::string("az") == cmdString) {
        azf = atof(&cmd[3]);
    } else if (std::string("to") == cmdString) {
        lastTeleopTime = teleopTime;
    } else if (std::string("dm") == cmdString) {
        lastDeadmanTime = deadmanTime;
    } else if (std::string("da") == cmdString) {
        disableAutoRun = atoi(&cmd[3]);
        if (disableAutoRun != lastDisableAutoRun) {
            sendAutoRun(disableAutoRun);
        }
        lastDisableAutoRun = disableAutoRun;
    }

    if ((teleopTime - lastTeleopTime) < 1000) {
        teleopActive = true;
    } else {
        teleopActive = false;
    }

    if ((deadmanTime - lastDeadmanTime) < 1000) {
        deadmanActive = true;
    } else {
        deadmanActive = false;
    }

    if (teleopActive) {
        if (axf > 5.0) axf = 5.0;
        else if (axf < -5.0) axf = -5.0;
        rotSpeed = maxRotSpeed * axf / 5.0;

        if (ayf < 0) vel = maxSpeed;
        else if (azf < 0) vel = -maxSpeed;
        else if (abs(azf - ayf) > 1.0) {
            velFwd = azf / (ayf + azf);
            velBackwd = ayf / (ayf + azf);
            if (velFwd > 0.55)
                vel = ((velFwd * 2) - 1.0) * maxSpeed;
            else if (velBackwd > 0.55)
                vel = -(((velBackwd * 2) - 1.0) * maxSpeed);
        } else {
            vel = 0.0;
            rotSpeed = 0.0;
        }
        sendTeleop(vel, rotSpeed);
    }

    if (deadmanActive) {
        if (disableAutoRun)
            sendAutoRun(disableAutoRun);
    }
}
void Roboremo::loop() {
    return; // Disable Bluetooth processing for now
    deadmanTime = millis();
    teleopTime = millis();

    if (SerialBT.available()) {
        char c = (char)SerialBT.read();
        cmd[cmdIndex] = c;
        if (cmdIndex < 99)
            cmdIndex++;
        cmd[cmdIndex] = '\0';

        if (c == '\n') {
            exeCmd();
            cmdIndex = 0;
        }
    }

    int now = millis();
    if (now > nextPingTime) {
        nextPingTime = millis() + 500;
        char hiString[] = "hi 1\n";
        for (int i = 0; i < (int)strlen(hiString); i++) {
            SerialBT.write(hiString[i]);
        }
        char autoString[50];
        int autoEnabled = disableAutoRun ? 0 : 1;
        sprintf(autoString, "ae %d\n", autoEnabled);
        for (int i = 0; i < (int)strlen(autoString); i++) {
            SerialBT.write(autoString[i]);
        }
    }
    delay(20);
}

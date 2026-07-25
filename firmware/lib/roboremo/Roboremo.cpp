#include "Roboremo.h"
#include <string>
#include <cstdlib>

#define RXD2 16
#define TXD2 17

Roboremo::Roboremo(geometry_msgs__msg__Twist & twist_msg, unsigned long & prev_cmd_time) : 
    cmdIndex(0),
    maxSpeed(0.7),
    maxRotSpeed(0.8),
    axf(0.0), ayf(0.0), azf(0.0),
    teleopTime(0),
    lastTeleopTime(0),
    teleopActive(false),
    disableAutoRun(0),
    lastDisableAutoRun(0),
    deadmanTime(0),
    lastDeadmanTime(0),
    deadmanActive(false),
    nextPingTime(0),
    twist_msg(twist_msg),
    prev_cmd_time(prev_cmd_time)
{
    memset(cmd, 0, sizeof(cmd));
}

void Roboremo::begin() {
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

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

void Roboremo::sendTeleop(float v, float rotSpeed) {
    // syslog(LOG_INFO, "%s: Teleop v: %0.2f r: %0.2f", __FUNCTION__, v, rotSpeed);
    twist_msg.linear.x = v;
    twist_msg.angular.z = rotSpeed;
    prev_cmd_time = millis();
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
    deadmanTime = millis();
    teleopTime = millis();

    if (Serial2.available()) {
        char c = (char)Serial2.read();
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
            Serial2.write(hiString[i]);
        }
        char autoString[50];
        int autoEnabled = disableAutoRun ? 0 : 1;
        sprintf(autoString, "ae %d\n", autoEnabled);
        for (int i = 0; i < (int)strlen(autoString); i++) {
            Serial2.write(autoString[i]);
        }
    }
}

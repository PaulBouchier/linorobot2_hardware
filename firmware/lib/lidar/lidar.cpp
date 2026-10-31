// Copyright (c) 2023 Thomas Chou
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Push lidar data to a UDP server, which will push laser scan message
// TODO: add PWM closed loop control the scan frequency
// TODO: add TCP for reliable data packet sequence
//
#include <Arduino.h>
#include "config.h"
#include "syslog.h"

#ifdef USE_LIDAR_UDP
#include <HardwareSerial.h>
#include <WiFiUdp.h>
#define BUFSIZE 512

HardwareSerial comm(LIDAR_SERIAL);
WiFiUDP udp;
uint8_t buf[BUFSIZE];

// Runs in the HardwareSerial uart_event_task, which has a small (2KB) stack.
// Do NOT do network I/O (syslog/UDP) here - it overflows the stack and panics.
volatile uint32_t lidar_rx_err_count = 0;
volatile uint32_t lidar_rx_err_by_type[6] = {0}; // indexed by hardwareSerial_error_t
volatile uint32_t lidar_rx_bytes = 0;
volatile uint32_t lidar_udp_pkts = 0;
void rx_err_callback(hardwareSerial_error_t err)
{
  lidar_rx_err_count++;
  if ((unsigned)err < 6) lidar_rx_err_by_type[err]++;
}

size_t len = 0;
void rx_callback(void)
{
  size_t cc = comm.read(buf + len, BUFSIZE - len);
  lidar_rx_bytes += cc;
  // syslog(LOG_INFO, "%s recv %d", __FUNCTION__, cc);
  // send larger UDP packet
  if ((len += cc) && (len > 128)) {
      udp.beginPacket(LIDAR_SERVER, LIDAR_PORT);
      udp.write(buf, len);
      udp.endPacket();
      lidar_udp_pkts++;
      // syslog(LOG_INFO, "%s send %d", __FUNCTION__, len);
      len = 0;
  }
}

void poweronLidar(void)
{
#ifdef LIDAR_POWEROFF
    digitalWrite(LIDAR_POWEROFF, LOW);
#endif
}

void poweroffLidar(void)
{
#ifdef LIDAR_POWEROFF
    digitalWrite(LIDAR_POWEROFF, HIGH);
#endif
}

void initLidar(void) {
  pinMode(LIDAR_RXD, INPUT);
#ifdef LIDAR_POWEROFF
  pinMode(LIDAR_POWEROFF, OUTPUT);
#endif
  poweronLidar();
  comm.setRxBufferSize(1024);
  comm.onReceiveError(rx_err_callback);
  comm.onReceive(rx_callback);
  comm.begin(LIDAR_BAUDRATE, SERIAL_8N1, LIDAR_RXD);
};

// Call from loop(): prints lidar UART/UDP counters every 2s (safe context for network I/O)
// Define LIDAR_STATS in the robot config to enable the periodic log.
void reportLidarStats(void)
{
#ifdef LIDAR_STATS
  static unsigned long last = 0;
  if (millis() - last < 2000) return;
  last = millis();
  Serial.printf("lidar: rx_bytes=%lu udp_pkts=%lu errs=%lu (break=%lu buf_full=%lu fifo_ovf=%lu frame=%lu parity=%lu) avail=%d\n",
    lidar_rx_bytes, lidar_udp_pkts, lidar_rx_err_count,
    lidar_rx_err_by_type[UART_BREAK_ERROR], lidar_rx_err_by_type[UART_BUFFER_FULL_ERROR],
    lidar_rx_err_by_type[UART_FIFO_OVF_ERROR], lidar_rx_err_by_type[UART_FRAME_ERROR],
    lidar_rx_err_by_type[UART_PARITY_ERROR], comm.available());
  syslog(LOG_INFO, "lidar rx_bytes=%lu udp_pkts=%lu errs=%lu frame=%lu break=%lu",
    lidar_rx_bytes, lidar_udp_pkts, lidar_rx_err_count,
    lidar_rx_err_by_type[UART_FRAME_ERROR], lidar_rx_err_by_type[UART_BREAK_ERROR]);
#endif
}
#else
void reportLidarStats(void) {};
void initLidar(void) {};
void poweronLidar(void) {};
void poweroffLidar(void) {};
#endif

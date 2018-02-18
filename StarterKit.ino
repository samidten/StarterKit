/*
  StarterKit.ino - Example app for Massive IoT
  Samid Tennakoon <samid.tennakoon@ericsson.com>
  Updated: 18 Jan 2018
*/

#include <RTCZero.h>
#include "uBloxModem.h"
#include <Wire.h>
#include <math.h>
#include <Sodaq_HTS221.h>

#define ADC_AREF 3.3f
#define BATVOLT_R1 4.7f // One v2
#define BATVOLT_R2 10.0f // One v2
#define BATVOLT_PIN A8

uBloxModem modem;
char apn[] = "hicard";
char server[] = "203.126.155.248"; // iotlab.zmalloc.org

RTCZero rtc;
bool wakeup = false;
Sodaq_HTS221 s1;
int counter = 0;
int once = 1;
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = A7;     // Grove - Temperature Sensor connect to A7

void setup() {

  pinMode(17,OUTPUT);
  digitalWrite(17, HIGH);
  pinMode(18,OUTPUT);
  digitalWrite(18, HIGH);
  
  Wire.begin();
  if (s1.init()) { // enable HTS221 sensor (temp/humidity)
      s1.enableSensor();
  }
  modem.init(apn, server);
      
  rtc.begin();
  rtc.setAlarmMinutes((rtc.getMinutes() + 1) % 60);
  rtc.enableAlarm(rtc.MATCH_MMSS);
  rtc.attachInterrupt(alarmTrigger);
  
}

void loop() {
  char v1[15];

  if (wakeup) {

    wakeup = false;
    
    int a = analogRead(pinTempSensor);
    float R = 1023.0/a-1.0;
    R = R0*R;
    float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; // convert to temperature via datasheet

    float voltage = (float)((ADC_AREF / 1.023) * (BATVOLT_R1 + BATVOLT_R2) / BATVOLT_R2 * (float)analogRead(BATVOLT_PIN)); // +35 offset from real
    
    counter = counter + 1;
    sprintf(v1, "%d;%f;%f", counter, voltage, temperature); // combine var1, var2, etc to a string
    digitalWrite(18, LOW);
    modem.publish(v1);
    digitalWrite(18, HIGH);
    delay(2000);
  }

  //Disable USB
  //USB->DEVICE.CTRLA.reg &= ~USB_CTRLA_ENABLE;

  rtc.setAlarmMinutes((rtc.getAlarmMinutes() + 1) % 60);
  rtc.standbyMode();    // Sleep until next alarm match

  //Enable USB
  //USB->DEVICE.CTRLA.reg |= USB_CTRLA_ENABLE;
}

void alarmTrigger() {
  wakeup = true;
}


// Sundown Light
// Todd Coffey
// Prototype in Silvia's bedroom 2013-07-05
#include "TimerOne.h"

TimerOne timer;

const int LED_pin = 9;
const int bogus_LED_pin = 6;
const int Button_pin = 4;
const int indicator_pin = 13;

bool lightOn = false;

// max light at 1008, min light at 176
// 2013-09-07 turned maxBright down from 800 to 400 for Silvia 
// Hopefully this will help her sleep.  
// 2013-09-08 It helped for one night. 
// 2013-10-06 She'd like it turned up.
const int16_t maxBright = 600; // resolution of TimerOne::pwm (10 bits) is 1024
const int16_t minBright = 176; 
const int16_t statusBright = 270;
const int timerPeriod = 1000; // 1000us = 1 kHz flicker

int sunsetLength = 900; // seconds (900 = 15 minutes)
int numLightPresses = 0;


const int debounceInterval = 5; // milliseconds
int debounceDigitalRead(int pin) {
  int lastResult = digitalRead(pin);
  unsigned long lastRead = millis();
  while (static_cast<unsigned long>(millis() - lastRead) < debounceInterval) {
    int result = digitalRead(pin);
    if (result != lastResult) {
      lastResult = result;
      lastRead = millis();
    }
  }
  return lastResult;
}


const int shortButtonInterval = 5; // milliseconds
const int mediumButtonInterval = 1000; // milliseconds
const int longButtonInterval = 10000; // milliseconds
// return values:  0 = OFF, 1 = short, 2 = medium, 3 = long
int buttonRead(int pin) {
  const int lastResult = digitalRead(pin);
  if (lastResult == HIGH) {
   return 0;
  }
  const unsigned long startTime = millis();
  unsigned long endTime = millis();
  while (static_cast<unsigned long>(millis() - startTime) < longButtonInterval) {
    int result = digitalRead(pin);
    if (result != lastResult) {
      break;
    }
    if (static_cast<unsigned long>(millis() - startTime) > mediumButtonInterval) {
      cancelLight();
    }
  }
  endTime = millis();
  if (static_cast<unsigned long>(endTime - startTime) >= longButtonInterval) {
    return 3;
  } else if (static_cast<unsigned long>(endTime - startTime) >= mediumButtonInterval) {
    return 2;
  } else if (static_cast<unsigned long>(endTime - startTime) >= shortButtonInterval) {
    return 1;
  }
  return 0;
}


// wait is in milliseconds
// last is the last time this function returned true
bool isTimeNow(unsigned long & last, unsigned wait) {
  unsigned long time = millis();
  if (static_cast<unsigned long>(time-last) > wait) {
    last = time;
    return true;
  }
  return false;
}

void turnLightOn(int currentLevel) {
  for (int i=currentLevel ; i < maxBright ; ++i) {
    timer.pwm(LED_pin,i);
    delay(1);
  }
  timer.pwm(LED_pin,maxBright);
}

void turnLightOff(int currentLevel) {
  for (int i=currentLevel ; i>0 ; --i) {
    timer.pwm(LED_pin,i);
    delay(1);
  }
  timer.pwm(LED_pin,0);
}

void waitForButtonDepress(int pin) {
  while (debounceDigitalRead(pin)==LOW) {
    delay(5);
  }
}

void setup() {
  //Serial.begin(9600);
  //Serial.println("Ready.");
  
  pinMode(bogus_LED_pin, INPUT);
  pinMode(LED_pin,OUTPUT); timer.pwm(LED_pin,0);
  pinMode(Button_pin,INPUT); 
  pinMode(indicator_pin,OUTPUT);
  
  timer.initialize(timerPeriod);
  timer.pwm(LED_pin, 0); 
}


unsigned long millisAtStartOfSunSet = millis();
unsigned long lastSunSetUpdate = millis();
const unsigned sunSetUpdateInterval = 1;
int lastSunSetLightLevel = 0;
void updateSunSet() {
  if (!lightOn) {
    return;
  }
  if (!isTimeNow(lastSunSetUpdate,sunSetUpdateInterval)) {
    return;
  }
  unsigned long currentMillis = millis();
  unsigned long delta = static_cast<unsigned long>(currentMillis - millisAtStartOfSunSet);
  unsigned long finalMillis = sunsetLength;
  finalMillis *= 1000;
  if (delta > finalMillis) {
    lightOn = false;
    lastSunSetLightLevel = 0;
    timer.pwm(LED_pin,lastSunSetLightLevel);
    digitalWrite(indicator_pin,LOW);
    //Serial.println("Turning off light");
  }
  else {
    double level = maxBright*1.0 + ((minBright-maxBright)*1.0*delta)/(finalMillis*1.0);
    lastSunSetLightLevel = level;
    //Serial.print(delta);
    //Serial.print("  ");
    //Serial.println(lastSunSetLightLevel);
    timer.pwm(LED_pin,lastSunSetLightLevel);
  }
}

void cancelLight() {
  if (lightOn) {
    lightOn = false;
    turnLightOff(lastSunSetLightLevel);
    lastSunSetLightLevel = 0;
    digitalWrite(indicator_pin,LOW);
  }
}

void displayReportMode() {
   digitalWrite(indicator_pin,HIGH);
   delay(1000);
   digitalWrite(indicator_pin,LOW);
   delay(1000);
   digitalWrite(indicator_pin,HIGH);
   delay(1000);
   digitalWrite(indicator_pin,LOW);
}

void loop() {
  
  int b = buttonRead(Button_pin);
  if (b == 1) // short press
  { 
    digitalWrite(indicator_pin,HIGH);
    ++numLightPresses;
    lightOn = true;
    turnLightOn(lastSunSetLightLevel);
    millisAtStartOfSunSet = millis();    
    //Serial.println("Turning on light.");
  } 
  else if (b == 2) // medium press
  {
    cancelLight();
  }
  else if (b == 3) // long press
  {
     cancelLight();
     displayReportMode();
     delay(1000);
     for (int i=0 ; i < numLightPresses ; ++i) {
       timer.pwm(LED_pin,statusBright);
       delay(500);
       timer.pwm(LED_pin,0);
       delay(500);
     }
     timer.pwm(LED_pin,0);
     displayReportMode();
  }
  
  updateSunSet();
}

//#include <EEPROM.h>
//#include <Arduino.h>  // for type definitions
//
//template <class T> int EEPROM_writeAnything(int ee, const T& value)
//{
//    const byte* p = (const byte*)(const void*)&value;
//    unsigned int i;
//    for (i = 0; i < sizeof(value); i++)
//          EEPROM.write(ee++, *p++);
//    return i;
//}
//
//template <class T> int EEPROM_readAnything(int ee, T& value)
//{
//    byte* p = (byte*)(void*)&value;
//    unsigned int i;
//    for (i = 0; i < sizeof(value); i++)
//          *p++ = EEPROM.read(ee++);
//    return i;
//}



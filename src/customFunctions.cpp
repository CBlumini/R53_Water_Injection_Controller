#include <CustomFunctions.h>
#include <Arduino.h>

extern bool outputFlag;
extern unsigned long currentTime;
extern unsigned long prevTime;
extern int outState;
extern bool outputFlag0;
extern bool outputFlag1;
extern bool outputFlag2;
extern bool outputFlag3;

void setupBuiltInLED(int freq, int ledChannel, int resolution, int ledPin){
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);
  Serial.println("LEDS set");
}


void ledOutput(int dutyCycle)
{
  Serial.println("running LED");
  ledcWrite(0, dutyCycle);
}

void cycleOutput(int cycleTime, int output, bool outputToggle)
{
  if (outputToggle)
    {
      currentTime = millis();
      if(currentTime - prevTime > 1000){
        prevTime = currentTime;
        Serial.println("time met");
        if(outState == LOW){
          Serial.println("going high");
          outState = HIGH;
        }
        else {
          outState = LOW;
        }
      
      digitalWrite(output, outState);
      }
      delay(10);
    }
    else {
      digitalWrite(output, LOW);
    }
}

void setFlags(int relayId, int relayState) {
  switch (relayId)
      {
      case 0:
        Serial.println("running code for led");
        if (relayState == 1) {
          outputFlag0 = true;
        }
        else {
          outputFlag0 = false;
        }
        break;
      case 1:
        Serial.println("running code for injector");
        if (relayState == 1) {
          outputFlag1 = true;
        }
        else {
          outputFlag1 = false;
        }
        break;
      case 2:
        Serial.println("running code for pump");
        if (relayState == 1) {
          outputFlag2 = true;
        }
        else {
          outputFlag2 = false;
        }
        break;
      case 3:
        Serial.println("running code for spare");
        if (relayState == 1) {
          outputFlag3 = true;
        }
        else {
          outputFlag3 = false;
        }
        break;
      default:
        break;
      }
}



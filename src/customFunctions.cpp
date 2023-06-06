#include <CustomFunctions.h>
#include <Arduino.h>

extern bool outputFlag;
extern unsigned long currentTime;
extern unsigned long prevTime;
extern int outState;

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
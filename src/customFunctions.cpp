#include <CustomFunctions.h>
#include <Arduino.h>

extern bool outputFlag;

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

void cycleOutput(int cycleTime, int output)
{
  Serial.println("cycling output");
  unsigned long currentTime = 0;
  unsigned long prevTime = 0;
  int outState = LOW;
  while (outputFlag)
  {
    currentTime = millis();
    if(currentTime - prevTime > cycleTime){
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
}
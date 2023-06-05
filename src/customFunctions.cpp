#include <CustomFunctions.h>
#include <Arduino.h>

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
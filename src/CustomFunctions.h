#ifndef CUSTOM_FUNCTIONS_H
#define CUSTOM_FUNCTIONS_H

#include <Arduino.h> // Include this if your functions use Arduino functions or types

// Declare your functions here:
void setupBuiltInLED(int freq, int ledChannel, int resolution, int ledPin);
void ledOutput(int dutyCycle);
void cycleOutput(int cycleTime, int output, bool outputToggle);
void setFlags(int relayId, int relayState);
int interpolateValues(int val, int* xMapVals, int* yMapVals, uint8_t size);

#endif // MY_FUNCTIONS_H
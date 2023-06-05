#ifndef CUSTOM_FUNCTIONS_H
#define CUSTOM_FUNCTIONS_H

#include <Arduino.h> // Include this if your functions use Arduino functions or types

// Declare your functions here:
void setupBuiltInLED(int freq, int ledChannel, int resolution, int ledPin);
void ledOutput(int dutyCycle);

// void setStartRPM(int startRPM);
// int getStartRPM();

// void setEndRPM(int endRPM);
// int getEndRPM();


#endif // MY_FUNCTIONS_H
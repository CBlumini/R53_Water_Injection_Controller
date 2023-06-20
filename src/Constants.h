#ifndef CONSTANTS_H
#define CONSTANTS_H

// Outputs
const int LEDPIN = 2;
const int VALVEPIN = 5;
const int PUMPRELAYPIN = 26;
const int SPAREOUTPUTPIN = 27;
const int PWM1_CH = 0;
const int PWM1_RES = 10;
const int PWM1_FREQ =100;
// Inputs
const int VOLTAGESENSORPIN = 33;
const int pushToInjectPin = 34;
const int flowMeterPin = 35;
const int INJECTORDUTYPIN = 36;
const int spareInputPin = 39;
// Ref vals
const float CARCHARGINETHRESHOLD = 13.0;
const float CARRUNNINGRPM = 500;

#endif
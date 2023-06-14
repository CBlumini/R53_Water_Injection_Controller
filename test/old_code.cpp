#define pulse_ip 7
int ontime,offtime,duty,valve_out;
float freq,period;
const int pumpTrigger = 35;
const int outStart = pumpTrigger+2;

//defin the pins
const int pumpPin = 5; //Digital pin for for pump relay
const int solenoidPin = 9; //set the pin for the meth control solenoid

#include <LiquidCrystal_I2C.h>

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  
   
void setup()
{
  //set the PWM frequency to roughly double 6000rpm, about 
  // Pins D9 and D10 - 122 Hz
  TCCR1A = 0b00000001; // 8bit
  TCCR1B = 0b00000100; // x256 phase correct

  //set the pin modes
  pinMode(pulse_ip,INPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);
  
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  lcd.begin(16, 4);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Fr:");
  lcd.setCursor(0,1);
  lcd.print("Du:");  
  lcd.setCursor(8,1);
  lcd.print("Out:");  


  
}



void loop()
{
  //read the duty
   ontime = pulseIn(pulse_ip,HIGH);
   offtime = pulseIn(pulse_ip,LOW);
   period = ontime+offtime;
   freq = 1000000.0/period;
   duty = (ontime/period)*100; 

 //trigger the pump
  if (duty > pumpTrigger)
{
    digitalWrite(pumpPin, LOW);
}
  else
{
    digitalWrite(pumpPin, HIGH);
}

  if (duty > outStart)
  {
    //multimap for solenoid valve, let you create arbitrary maps
    // out[] holds the values wanted in cm
    int out[] = {255, 225, 128};

    // in[] hold the input values from the duty. Use this to set the start and stop point, should be linear (use excel)
    // note: the in array should have increasing values
    int in[]  = {outStart, 68, 100};
    valve_out = multiMap(duty, in, out, 3);

    //output the PWM for the solenoid valve
    //valve_out = map(duty, 50, 100, 0, 255);
    analogWrite(solenoidPin, valve_out);
  }
  else
  {
    valve_out=0;
    analogWrite(solenoidPin, valve_out);
  }

   //dispay the reading
   lcd.setCursor(3,0); 
   lcd.print((int)freq);
   lcd.print("Hz");
   lcd.setCursor(3,1);
   lcd.print(duty); 
   lcd.print('%'); 
   lcd.setCursor(12,1);
   lcd.print(valve_out); 
   delay(250);
}

//declare the multimap function
// note: the xMapVals array should have increasing values
int multiMap(int val, int* xMapVals, int* yMapVals, uint8_t size) {
    if (val <= xMapVals[0]) return yMapVals[0];
    if (val >= xMapVals[size - 1]) return yMapVals[size - 1];
    uint8_t pos = 1;
    while (val > xMapVals[pos]) pos++;
    if (val == xMapVals[pos]) return yMapVals[pos];
    return (val - xMapVals[pos - 1]) * (yMapVals[pos] - yMapVals[pos - 1]) / (xMapVals[pos] - xMapVals[pos - 1]) + yMapVals[pos - 1];
}

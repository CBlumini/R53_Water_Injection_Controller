# R53 Water Injection Controller
### Overview
This project is designed to use an inexpensive ESP32 microcontroller to control a water injection system on a 2005 Mini Cooper S (R53). On forced induction cars such as the Mini water injection after the supercharger can be used to greatly reduce charge temperature increasing power and while reducing the chance of engine knock.

The control is designed to scale the amount of water being injected based on engine speed and load, which correlates closely to the mass of air going into the engine. It does this by monitoring the fuel injection system and measuring injector duty cycle and frequency, which can be converted into load and engine speed.

The controller has a web based front end where the user can set break points to scale the amount of water injected based on how the engine responds to different amounts. Since engine behavior is quite non-linear this allows for a higher percent mass of water at peak torque where it is most needed while allowing it to scale down at higher speeds improving power and conservig water.

Water is stored in a tank in the trunk of the vehicle and is sent to the front of the vehicle by a high pressure diaphram pump running in bypass mode. The controller is designed to turn the pump on before injection is requested to make sure fluid is always available. Injection is controlled by a solenoid valve in the engine compartment that is PWMed based on the amount of water the controller requests. 

The controller has several safeties to make sure it does not inject at the wrong time. Most importantly it looks at vehicle voltage to make sure the engine is running, which avoids potentially hydrolocking an engine that is off (a problem with the old Aquamist Controller)

Development was supported by PlatformIO and Wokwi ESP32 emulator.
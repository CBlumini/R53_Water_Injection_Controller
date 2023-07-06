#include <Arduino.h>
#include <map>
#include <string>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <secrets.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <CustomFunctions.h>
#include <Constants.h>
#include <Preferences.h>
#include <typeinfo>
#include <iostream>
#include <webpage.h>
// #include <PreferencesManager.h>

// Set how you want to connect
// #define USE_INTRANET
// #define USE_AP
#define SIMULATOR
#define USE_PREFENCES

// once  you are read to go live these settings are what you client will connect to
#define AP_SSID "Water Inject"
#define AP_PASS NULL

// #define NORMALLY_OPEN true

// Networking Setup
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;
// variable for the IP reported when you connect to your homes intranet (during debug mode)
IPAddress actualIP;

AsyncWebServer server(80);

int actuatorOutputs[] = {LEDPIN, VALVEPIN, PUMPRELAYPIN, SPAREOUTPUTPIN};

int injectionStartRPM = 0;
int injectionEndRPM = 0;
float scalingFactor = 0.0;

bool outputFlag0 = false;
bool outputFlag1 = false;
bool outputFlag2 = false;
bool outputFlag3 = false;
bool carCharging = false;
bool carRunning = false;
bool safeToInject = false;

unsigned long currentTime = 0;
unsigned long prevTime = 0;
int outState = LOW;

struct SpeedDemandStruct {
  int speed;
  int speedMult;
  int dutyCycle;
  int methanol;
};

std::map<std::string, SpeedDemandStruct> demandMap = {
  {"bp1", {3000, 1, 100, 0}},
  {"bp2", {3500, 1, 100, 0}},
  {"bp3", {4000, 1, 100, 0}},
  {"bp4", {4500, 1, 100, 0}},
  {"bp5", {5000, 1, 100, 0}},
  {"bp6", {5500, 1, 100, 0}},
  {"bp7", {6000, 1, 100, 0}},
  {"bp8", {6500, 1, 100, 0}},
  {"bp9", {7000, 1, 100, 0}}
};


///////////////////////// WEB PAGE CODE ///////////////////
#pragma region
// const char index_html[] WEBPAGE;

#pragma endregion
//////////////////////// END WEB CODE /////////////////////


////////////////// SUPPORT FUNCTIONS AND CLASS DECLARATIONS///////////
void printMap(const std::map<std::string, SpeedDemandStruct>& demandMap) {
    for (auto const& pair: demandMap) {
        Serial.print("Key: ");
        Serial.print(pair.first.c_str());
        Serial.print(" Speed: ");
        Serial.print(pair.second.speed);
        Serial.print(" SpeedMult: ");
        Serial.print(pair.second.speedMult);
        Serial.print(" Duty: ");
        Serial.print(pair.second.dutyCycle);
        Serial.print(" Meth: ");
        Serial.println(pair.second.methanol);
    }
}

//////////////// END SUPPORT FUNCTIONS //////////////////

Preferences preferences;
void setup()
{
//////////////////////// WIFI SETUP //////////////////
#pragma region
#ifdef USE_INTRANET
  WiFi.begin(LOCAL_SSID, LOCAL_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  actualIP = WiFi.localIP();
#endif

  // if you don't have #define USE_INTRANET, use AP Mode
#ifdef USE_AP
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  actualIP = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(actualIP);
#endif

#ifdef SIMULATOR
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
#endif
  ////////////// END WIFI SETUP//////////
#pragma endregion

  ///////////////// MAIN CODE ///////////////////

  Serial.begin(115200);

  // Set up the output pins
  pinMode(LEDPIN, OUTPUT);
  pinMode(VALVEPIN, OUTPUT);
  pinMode(PUMPRELAYPIN, OUTPUT);
  pinMode(SPAREOUTPUTPIN, OUTPUT);
  pinMode(VOLTAGESENSORPIN, INPUT);
  pinMode(INJECTORDUTYPIN, INPUT);


  // ledcAttachPin(VALVEPIN, PWM1_CH);
  // ledcSetup(PWM1_CH, PWM1_FREQ, PWM1_RES);


  // Load the values stored in preferences into the demand map
  #ifdef USE_PREFERENCES
  preferences.begin("my-app", false);
  for (int i = 1; i <= 9; i++)
  {
    String baseKey = "bp" + String(i);
    std::string baseKeyStd = baseKey.c_str();

      // bpStoreSpeed = "speed" + pair.first;
      // bpStoreSpeedMult = "speedMult" + pair.first;
      // bpStoreDuty = "duty" + pair.first;
      // bpStoreMethanol = "methanol" + pair.first;

    String speedKey = "speed" + baseKey;
    String speedMultKey = "speedMult" + baseKey;
    String dutyKey = "duty" + baseKey;
    String methanolKey = "methanol" + baseKey;

    demandMap[baseKeyStd].speed = preferences.getInt(speedKey.c_str(), 0);
    demandMap[baseKeyStd].speedMult = preferences.getInt(speedMultKey.c_str(), 0);
    demandMap[baseKeyStd].dutyCycle = preferences.getInt(dutyKey.c_str(), 0);
    demandMap[baseKeyStd].methanol = preferences.getInt(methanolKey.c_str(), 0);
  }
  preferences.end();
  #endif



  ///////////////SETUP ROUTES////////////
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", WEBPAGE); });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    int relayId;
    int relayState;
    if (request->hasParam("relay") & request->hasParam("state")) {
      relayId = request->getParam("relay")->value().toInt();
      relayState = request->getParam("state")->value().toInt();
      setFlags(relayId, relayState);
    }

    Serial.println(relayId + relayState);
    request->send(200, "text/plain", "OK"); });


  server.on("/getOutputStates", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    DynamicJsonDocument doc(1024);
    for (int i = 0; i < 4; i++)
    {
      doc[String(i)] = digitalRead(actuatorOutputs[i]);
    }
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response); });

  server.on("/getDemandMap", HTTP_GET, [](AsyncWebServerRequest *request){
    DynamicJsonDocument doc(1024);
    for (auto const& pair: demandMap) {
      doc[pair.first.c_str()]["speed"] = pair.second.speed;
      doc[pair.first.c_str()]["speedMult"] = pair.second.speedMult;
      doc[pair.first.c_str()]["dutyCycle"] = pair.second.dutyCycle;
      doc[pair.first.c_str()]["methanol"] = pair.second.methanol;
    }
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
    

server.on("/updateDemands", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) 
  {
    preferences.begin("my-app", false);
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
      // Expecting a JSON object so check it's the correct type
    if (!doc.is<JsonObject>()) {
      Serial.println(F("JSON is not an object"));
      return;
    }

    JsonObject obj = doc.as<JsonObject>();

    const char* bpTemp;

    // PRINT ALL THE DATA
    for(JsonPair bpPair : obj) {
      bpTemp = bpPair.key().c_str();
      // Serial.println(bpTemp);
      JsonObject bp = bpPair.value().as<JsonObject>();
      SpeedDemandStruct &demandStruct = demandMap[bpTemp];

      for (JsonPair keyVal : bp) {
        int value = keyVal.value().as<int>();
        // Serial.print(keyVal.key().c_str());
        // Serial.print(": ");
        // Serial.println(value);

        if (keyVal.key() == "speed") {
          demandStruct.speed = value;
        }
        else if (keyVal.key() == "speedMult") {
          demandStruct.speedMult = value;
        }
        else if (keyVal.key() == "dutyCycle") {
          demandStruct.dutyCycle = value;
        }
        else if (keyVal.key() == "methanol") {
          demandStruct.methanol = value;
        }
      }
    };

    // PRINT THE MAP
    printMap(demandMap);

    // update the preference values
    std::string bpStoreSpeed = "";
    std::string bpStoreSpeedMult = "";
    std::string bpStoreDuty = "";
    std::string bpStoreMethanol = "";
    for (auto const& pair: demandMap) {
      bpStoreSpeed = "speed" + pair.first;
      bpStoreSpeedMult = "speedMult" + pair.first;
      bpStoreDuty = "duty" + pair.first;
      bpStoreMethanol = "methanol" + pair.first;
      preferences.putInt(bpStoreSpeed.c_str(), pair.second.speed);
      preferences.putInt(bpStoreSpeedMult.c_str(), pair.second.speedMult);
      preferences.putInt(bpStoreDuty.c_str(), pair.second.dutyCycle);
      preferences.putInt(bpStoreMethanol.c_str(), pair.second.methanol);

    }
    preferences.end();

  });

  // might make sense to do this
  // 1 validate numbers
  // 2 save to prefs
  // 3 trigger a fucntion that loads preferences
  // 4 reuse that same function to load the prefs on startup
  // Note: don't seriously need a function to write because that should only ever be modified due to this post request

 // some test
// Start server
server.begin();

};
////////////// END MAIN SETUP CODE /////////////////


void loop()
{
  cycleOutput(1000, LEDPIN, outputFlag0);
  cycleOutput(1000, VALVEPIN, outputFlag1);
  cycleOutput(1000, PUMPRELAYPIN, outputFlag2);

#pragma region
#ifdef USE_INTRANET
  // get stuff about the injectors
  long onTime = pulseIn(INJECTORDUTYPIN, HIGH);
  long offTime = pulseIn(INJECTORDUTYPIN, LOW);
  long period = onTime+offTime;
  float freq = 10000000/period;
  int rpms = freq*2*60; // two revs per pulse then seconds to mins
  int duty = (onTime/period)*100;
#endif
#ifdef USE_AP
  // get stuff about the injectors
  long onTime = pulseIn(INJECTORDUTYPIN, HIGH);
  long offTime = pulseIn(INJECTORDUTYPIN, LOW);
  long period = onTime+offTime;
  float freq = 10000000/period;
  int rpms = freq*2*60; // two revs per pulse then seconds to mins
  int duty = (onTime/period)*100;
#endif
#ifdef SIMULATOR
  // 3.3v and 4096 counts... 4095/3.3
  float carVoltageRaw = analogRead(VOLTAGESENSORPIN);
  float carVoltage = (carVoltageRaw/1240.90) * 10; // assume 10x divider
  // TODO: Figure out why this isnt scaling right
  float period = analogRead(INJECTORDUTYPIN)/4095;
  int rpms = period*7000;
  int duty = period;
#endif
#pragma endregion
  // check that the car is on and charging
  if (carVoltage > CARCHARGINETHRESHOLD){
    Serial.println("Voltage met");
    Serial.println(carVoltage);
    carCharging = true;
  } else {
    carCharging = false;
  }

  // check the car is running
  if (rpms > CARRUNNINGRPM) {
    Serial.println("speed met");
    Serial.println(period);
    Serial.println(rpms);
    carRunning = true;
  } else {
    carRunning = false;
  }

  // enable injection
  if (carRunning && carCharging){
    // turn on the pump
    Serial.print("injecting");
    digitalWrite(PUMPRELAYPIN, HIGH);

    // get RPM scaling
    // float rpmScalar = interpolateValues(rpms, , 0, 1);

  } else{
    // do nothing 
  }


  //TODO: Implement simple voltage check
  //TODO: Implement pump actuation (lowest speed bp with meth demand -100rpm)
  //TODO: Implement duty cycle read
  //TODO: Implement valve output
  delay(1);
};



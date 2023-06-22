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
// #include <PreferencesManager.h>

// Set how you want to connect
// #define USE_INTRANET
// #define USE_AP
#define SIMULATOR
// #define USE_PREFENCES

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
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      text-align: center;
    }

    h2 {
      font-size: 2.5rem;
    }

    p {
      font-size: 2.5rem;
    }

    body {
      max-width: 600px;
      margin: 0px auto;
      padding-bottom: 25px;
    }

    label {
      font-size: small;
    }

    .switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
    }

    .switch input {
      display: none;
    }

    .slider {
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      border-radius: 17px;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 26px;
      width: 26px;
      left: 4px;
      bottom: 4px;
      background-color: #fff;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 34px;
    }

    input:checked+.slider {
      background-color: #2196F3;
    }

    input:checked+.slider:before {
      -webkit-transform: translateX(26px);
      -ms-transform: translateX(26px);
      transform: translateX(26px);
    }

    .flex-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
    }
  </style>
</head>

<body>
  <h3>Blumini Injects Test and Setup Page</h3>
  <div class="flex-container">
    <h4>LED Test</h4>
    <label class="switch" for="0">
      <input type="checkbox" id="0" onchange="toggleCheckbox(this)">
      <span class="slider">
      </span>
    </label>
    <h4>Injector Test</h4>
    <label class="switch" for="1">
      <input type="checkbox" id="1" onchange="toggleCheckbox(this)">
      <span class="slider">
      </span>
    </label>
    <h4>Pump Test</h4>
    <label class="switch" for="2">
      <input type="checkbox" id="2" onchange="toggleCheckbox(this)">
      <span class="slider">
      </span>
    </label>
    <h4>Spare Output Test</h4>
    <label class="switch" for="3">
      <input type="checkbox" id="3" onchange="toggleCheckbox(this)">
      <span class="slider">
      </span>
    </label>
  </div>
  <div class="flex-container">
    <h4>Enter Speed, Duty Cycle and Demand Values:</h4>
    <h5> The speed is first checked where a multiplier factor is applied then
      the injector duty cycle is checked and the output value is multiplied by the 
      speed multiplier
    </h5>

    <div style="display: flex;">
      <div>
        <h5>Speed Values</h5>
        <input type="text" id="sp1" placeholder="Enter speed value" onchange="updateSpeed('1')">
        <input type="text" id="sp2" placeholder="Enter speed value" onchange="updateSpeed('2')">
        <input type="text" id="sp3" placeholder="Enter speed value" onchange="updateSpeed('3')">
        <input type="text" id="sp4" placeholder="Enter speed value" onchange="updateSpeed('4')">
        <input type="text" id="sp5" placeholder="Enter speed value" onchange="updateSpeed('5')">
        <input type="text" id="sp6" placeholder="Enter speed value" onchange="updateSpeed('6')">
        <input type="text" id="sp7" placeholder="Enter speed value" onchange="updateSpeed('7')">
        <input type="text" id="sp8" placeholder="Enter speed value" onchange="updateSpeed('8')">
        <input type="text" id="sp9" placeholder="Enter speed value" onchange="updateSpeed('9')">
      </div>
      <div>
        <h5>Speed Multipliers</h5>
        <input type="text" id="spm1" placeholder="Enter speed mult value" onchange="updateSpeedMult('1')">
        <input type="text" id="spm2" placeholder="Enter speed mult value" onchange="updateSpeedMult('2')">
        <input type="text" id="spm3" placeholder="Enter speed mult value" onchange="updateSpeedMult('3')">
        <input type="text" id="spm4" placeholder="Enter speed mult value" onchange="updateSpeedMult('4')">
        <input type="text" id="spm5" placeholder="Enter speed mult value" onchange="updateSpeedMult('5')">
        <input type="text" id="spm6" placeholder="Enter speed mult value" onchange="updateSpeedMult('6')">
        <input type="text" id="spm7" placeholder="Enter speed mult value" onchange="updateSpeedMult('7')">
        <input type="text" id="spm8" placeholder="Enter speed mult value" onchange="updateSpeedMult('8')">
        <input type="text" id="spm9" placeholder="Enter speed mult value" onchange="updateSpeedMult('9')">
      </div>
      <div>
        <h5>Duty Cycle Values</h5>
        <input type="text" id="dc1" placeholder="Enter speed value" onchange="updateDuty('1')">
        <input type="text" id="dc2" placeholder="Enter speed value" onchange="updateDuty('2')">
        <input type="text" id="dc3" placeholder="Enter speed value" onchange="updateDuty('3')">
        <input type="text" id="dc4" placeholder="Enter speed value" onchange="updateDuty('4')">
        <input type="text" id="dc5" placeholder="Enter speed value" onchange="updateDuty('5')">
        <input type="text" id="dc6" placeholder="Enter speed value" onchange="updateDuty('6')">
        <input type="text" id="dc7" placeholder="Enter speed value" onchange="updateDuty('7')">
        <input type="text" id="dc8" placeholder="Enter speed value" onchange="updateDuty('8')">
        <input type="text" id="dc9" placeholder="Enter speed value" onchange="updateDuty('9')">
      </div>
      <div style="margin-right: 10px;">
      <h5> Methanol Values </h5>
        <input type="text" id="mo1" placeholder="Enter value for break point" onchange="updateMethanol('1')">
        <input type="text" id="mo2" placeholder="Enter value for break point" onchange="updateMethanol('2')">
        <input type="text" id="mo3" placeholder="Enter value for break point" onchange="updateMethanol('3')">
        <input type="text" id="mo4" placeholder="Enter value for break point" onchange="updateMethanol('4')">
        <input type="text" id="mo5" placeholder="Enter value for break point" onchange="updateMethanol('5')">
        <input type="text" id="mo6" placeholder="Enter value for break point" onchange="updateMethanol('6')">
        <input type="text" id="mo7" placeholder="Enter value for break point" onchange="updateMethanol('7')">
        <input type="text" id="mo8" placeholder="Enter value for break point" onchange="updateMethanol('8')">
        <input type="text" id="mo9" placeholder="Enter value for break point" onchange="updateMethanol('9')">
      </div>

    </div>
    <button onclick="submitData()">Submit Data</button>
  </div>
  <script>

    var demandMapCli = {
      "bp1": {speed: 0, speedMult: 0, dutyCycle: 0, methanol: 0},
      "bp2": {speed: 0, speedMult: 0, dutyCycle: 0, methanol: 0},
      "bp3": {speed: 0, speedMult: 0, dutyCycle: 0, methanol: 0},
      "bp4": {speed: 0, speedMult: 0, dutyCycle: 0, methanol: 0},
      "bp5": {speed: 0, speedMult: 0, dutyCycle: 0, methanol: 0},
      "bp6": {speed: 0, speedMult: 0, dutyCycle: 0, methanol: 0},
      "bp7": {speed: 0, speedMult: 0, dutyCycle: 0, methanol: 0},
      "bp8": {speed: 0, speedMult: 0, dutyCycle: 0, methanol: 0},
      "bp9": {speed: 0, speedMult: 0, dutyCycle: 0, methanol: 0},
    };

    var numOutputs = 4;

    /////////// MAP UPDATE FUNCS ////////////
    function updateSpeed(bp){
      let value = Number(document.getElementById("sp"+bp).value);
      if (!Number.isInteger(Number(value))) {
        alert("Must be integer")
        return;
      }
      //TODO: add check for less than 100
      //TODO: add check for 0 or greater
      demandMapCli["bp"+bp].speed = value
    }

    function updateSpeedMult(bp){
      let value = Number(document.getElementById("spm"+bp).value);
      if (!Number.isInteger(Number(value))) {
        alert("Must be integer")
        return;
      }
      //TODO: add check for less than 100
      //TODO: add check for 0 or greater
      demandMapCli["bp"+bp].speedMult = value
    }

    function updateDuty(bp){
      let value = Number(document.getElementById("dc"+bp).value);
      if (!Number.isInteger(Number(value))) {
        alert("Must be integer")
        return;
      }
      //TODO: add check for less than 100
      //TODO: add check for 0 or greater
      demandMapCli["bp"+bp].dutyCycle = value
    }

    function updateMethanol(bp){
      let value = Number(document.getElementById("mo"+bp).value);
      if (!Number.isInteger(Number(value))) {
        alert("Must be integer")
        return;
      }
      //TODO: add check for less than 100
      //TODO: add check for 0 or greater
      demandMapCli["bp"+bp].methanol = value
    }

    function submitData() {
      console.log("User data to be sent to server: ", demandMapCli);

      let xhr = new XMLHttpRequest();
      xhr.open("POST", "/updateDemands", true);
      xhr.setRequestHeader("Content-Type", "application/json");

      xhr.onload = function () {
      if (xhr.status == 200) {
        alert("Data successfully sent to the server.");
      } else {
        alert("Failed to send data. Error: " + xhr.status);
      }
      };

      xhr.onerror = function () {
        alert("Request failed.");
      };

      xhr.send(JSON.stringify(demandMapCli));
    }
    /////////// END MAP UPDATE FUNCS ////////////

    window.onload = function () {
      // first request, get the state of the outputs
      let xhr1 = new XMLHttpRequest();
      xhr1.open("GET", "/getOutputStates", true);
      xhr1.onload = function () {
        if (xhr1.status == 200) {
          let relayStates = JSON.parse(xhr1.responseText);
          for (let i = 0; i < numOutputs; i++) {
            let checkbox = document.getElementById(i);
            checkbox.checked = relayStates[i.toString()] === "HIGH";
          }
        }
      };
      xhr1.send();

      // second request to get the values from the demand map
      // use this to populate the values in the text boxes when the page loads
      // so the user knows what the existing values are
      let xhr2 = new XMLHttpRequest();
      xhr2.open("GET", "/getDemandMap", true);

      xhr2.onload = function () {
        if (xhr2.status == 200) {
          let demandVals = JSON.parse(xhr2.responseText);
          let counter = 1;
          //console.log(demandVals)
          Object.keys(demandVals).forEach((key)=> {
            // update our local copy of the demand map
            demandMapCli[key] = demandVals[key]

            // update the values in the input boxes
            document.getElementById("sp" + (counter)).placeholder = `${demandMapCli[key].speed}`
            document.getElementById("spm" + (counter)).placeholder = `${demandMapCli[key].speedMult}`
            document.getElementById("dc" + (counter)).placeholder = `${demandMapCli[key].dutyCycle}`
            document.getElementById("mo" + (counter)).placeholder = `${demandMapCli[key].methanol}`
            counter++;
          });
        } else {
              console.log("Failed to load data")
            }
      }
      xhr2.onerror = function () {
        console.log("Request Failed")
      };
        
      xhr2.send();
    }


    function toggleCheckbox(element) {
      console.log("checkbox toggled")
      let xhr = new XMLHttpRequest();
      if (element.checked) {
        xhr.open("GET", "/update?relay=" + element.id + "&state=1", true);
      } else {
        xhr.open("GET", "/update?relay=" + element.id + "&state=0", true);
      }
      xhr.send();
    }
  </script>
</body>

</html>
)rawliteral";

#pragma endregion
//////////////////////// END WEB CODE /////////////////////


////////////////// SUPPORT FUNCTIONS AND CLASS DECLARATIONS///////////
void printMap() {
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
            { request->send_P(200, "text/html", index_html); });

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
    printMap();

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

  } else{
    // do nothing 
  }


  //TODO: Implement simple voltage check
  //TODO: Implement pump actuation (lowest speed bp with meth demand -100rpm)
  //TODO: Implement duty cycle read
  //TODO: Implement valve output
  delay(1);
};



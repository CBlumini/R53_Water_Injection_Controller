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

int actuatorOutputs[] = {ledPin, valvePin, pumpRelayPin, spareOutputPin};

int injectionStartRPM = 0;
int injectionEndRPM = 0;
float scalingFactor = 0.0;

bool outputFlag0 = false;
bool outputFlag1 = false;
bool outputFlag2 = false;
bool outputFlag3 = false;

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
  {"breakPoint1", {3000, 1, 100, 0}},
  {"breakPoint2", {3500, 1, 100, 0}},
  {"breakPoint3", {4000, 1, 100, 0}},
  {"breakPoint4", {4500, 1, 100, 0}},
  {"breakPoint5", {5000, 1, 100, 0}},
  {"breakPoint6", {5500, 1, 100, 0}},
  {"breakPoint7", {6000, 1, 100, 0}},
  {"breakPoint8", {6500, 1, 100, 0}},
  {"breakPoint9", {7000, 1, 100, 0}}
};


// // Access value
// int value = demandMap["key_3000"];
// // Modify value
// demandMap["key_3000"] = 100;

// struct ValueStruct {
//     int value1;
//     int value2;
// };

// std::map<std::string, ValueStruct> myMap;

// myMap["key_3000"] = {3000, 0};

// // Accessing the values
// int firstValue = myMap["key_3000"].value1;
// int secondValue = myMap["key_3000"].value2;

// // Modifying the values
// myMap["key_3000"].value1 = 4000;
// myMap["key_3000"].value2 = 100;



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

    var dataPoints = {
      "breakpoint1": {speed: 0, speedMultiplier: 0, dutyCycle: 0, methanol: 0},
      "breakpoint2": {speed: 0, speedMultiplier: 0, dutyCycle: 0, methanol: 0},
      "breakpoint3": {speed: 0, speedMultiplier: 0, dutyCycle: 0, methanol: 0},
      "breakpoint4": {speed: 0, speedMultiplier: 0, dutyCycle: 0, methanol: 0},
      "breakpoint5": {speed: 0, speedMultiplier: 0, dutyCycle: 0, methanol: 0},
      "breakpoint6": {speed: 0, speedMultiplier: 0, dutyCycle: 0, methanol: 0},
      "breakpoint7": {speed: 0, speedMultiplier: 0, dutyCycle: 0, methanol: 0},
      "breakpoint8": {speed: 0, speedMultiplier: 0, dutyCycle: 0, methanol: 0},
      "breakpoint9": {speed: 0, speedMultiplier: 0, dutyCycle: 0, methanol: 0},
    };

    var numOutputs = 4;

    function updateSpeed(breakpoint){
      let value = Number(document.getElementById("sp"+breakpoint).value);
      if (!Number.isInteger(Number(value))) {
        alert("Must be integer")
        return;
      }
      //TODO: add check for less than 100
      //TODO: add check for 0 or greater
      dataPoints["breakpoint"+breakpoint].speed = value
    }

    function updateSpeedMult(breakpoint){
      let value = Number(document.getElementById("spm"+breakpoint).value);
      if (!Number.isInteger(Number(value))) {
        alert("Must be integer")
        return;
      }
      //TODO: add check for less than 100
      //TODO: add check for 0 or greater
      dataPoints["breakpoint"+breakpoint].speedMultiplier = value
    }

    function updateDuty(breakpoint){
      let value = Number(document.getElementById("dc"+breakpoint).value);
      if (!Number.isInteger(Number(value))) {
        alert("Must be integer")
        return;
      }
      //TODO: add check for less than 100
      //TODO: add check for 0 or greater
      dataPoints["breakpoint"+breakpoint].dutyCycle = value
    }

    function updateMethanol(breakpoint){
      let value = Number(document.getElementById("dp"+breakpoint).value);
      if (!Number.isInteger(Number(value))) {
        alert("Must be integer")
        return;
      }
      //TODO: add check for less than 100
      //TODO: add check for 0 or greater
      dataPoints["breakpoint"+breakpoint].methanol = value
    }

    function submitData() {
      console.log("User data to be sent to server: ", dataPoints);

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

      xhr.send(JSON.stringify(dataPoints));
    }

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

      // second request get the state of the injection setpoints
      let xhr2 = new XMLHttpRequest();
      xhr2.open("GET", "/getInjectionDemands", true);

      xhr2.onload = function () {
        if (xhr2.status == 200) {
          let demandVals = JSON.parse(xhr2.responseText);
          let demandKeys = ["key_3000", "key_3500", "key_4000", "key_4500", "key_5000", "key_5500", "key_6000", "key_6500", "key_7000"]

          demandKeys.forEach((key) => {
            let input = document.getElementById(key);
            if (input && demandVals.hasOwnProperty(key)) {
              input.placeholder = `Current ${demandVals[key]}`;
              injectionDemands[key] = demandVals[key];
            }
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
    function updateStartRPM() {
      let startRPM = document.getElementById('var1').value;
      // Validation
      if (!Number.isInteger(Number(startRPM))) {
        alert("Must be a whole number");
        return;
      }
      let xhr = new XMLHttpRequest();
      xhr.open("GET", "/updateStartRPM?startRPM=" + startRPM);

      // let the user know it worked
      xhr.onload = () => {
        if (xhr.status == 200) {
          alert("Updated Start RPM to", xhr.responseText)
        } else {
          alert("Failed to update Start RPM, Error: ", xhr.status)
        }
      };

      // Error if the request deosnt go through
      xhr.onerror = () => {
        alert("Request failed.");
      };
      xhr.send();
    }
    function updateEndRPM() {
      let endRPM = document.getElementById('var2').value;
      // Validation
      if (!Number.isInteger(Number(endRPM))) {
        alert("Must be a whole number");
        return;
      }
      let xhr = new XMLHttpRequest();
      xhr.open("GET", "/updateEndRPM?endRPM=" + endRPM);

      // let the user know it went througg
      xhr.onload = () => {
        if (xhr.status == 200) {
          alert("Updated End RPM")
        } else {
          alert("Failed to update End RPM, Error: ", xhr.status)
        }
      };

      // Error if the request doesnt go through
      xhr.onerror = () => {
        alert("Request failed.");
      };
      xhr.send();
    }
  </script>
</body>

</html>
)rawliteral";

#pragma endregion
//////////////////////// END WEB CODE /////////////////////
// I think I got this code from the wifi example


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

// void printJson(JsonObject){
//   for (JsonObject::iterator it = doc.begin())
// }

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
  pinMode(ledPin, OUTPUT);
  pinMode(valvePin, OUTPUT);
  pinMode(pumpRelayPin, OUTPUT);
  pinMode(spareOutputPin, OUTPUT);

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


  server.on("/getStartEnd", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.println("getting start and end");
    DynamicJsonDocument doc(1024);

    doc["startRPM"] = preferences.getUInt("start", 0);
    Serial.println(preferences.getString("start"));
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

    const char* breakPointTemp;
    int speedTemp = 0;
    int speedMultTemp = 0;
    int dutyCylceTemp = 0;
    int methTemp = 0;


    // PRINT ALL THE DATA
    for(JsonPair breakPointPair : obj) {
      breakPointTemp = breakPointPair.key().c_str();
      Serial.println(breakPointTemp);
      JsonObject breakPoint = breakPointPair.value().as<JsonObject>();
      SpeedDemandStruct &demandStruct = demandMap[breakPointTemp];

      for (JsonPair keyVal : breakPoint) {
        int value = keyVal.value().as<int>();
        Serial.print(keyVal.key().c_str());
        Serial.print(": ");
        Serial.println(value);

        if (keyVal.key() == "speed") {
          demandStruct.speed = value;
        }
        else if (keyVal.key() == "speedMultiplier") {
          demandStruct.speedMult = value;
        }
        else if (keyVal.key() == "dutyCycle") {
          demandStruct.dutyCycle = value;
        }
        else if (keyVal.key() == "dutyCycle") {
          demandStruct.dutyCycle = value;
        }
      }
    };

    // update the preference values
    std::string breakPointStoreSpeed = "";
    std::string breakPointStoreSpeedMult = "";
    std::string breakPointStoreDuty = "";
    std::string breakPointStoreMethanol = "";
    for (auto const& pair: demandMap) {
      breakPointStoreSpeed = "speed" + pair.first;
      breakPointStoreSpeedMult = "speedMult" + pair.first;
      breakPointStoreDuty = "duty" + pair.first;
      breakPointStoreMethanol = "methanol" + pair.first;
      preferences.putInt(breakPointStoreSpeed.c_str(), pair.second.speed);
      preferences.putInt(breakPointStoreSpeedMult.c_str(), pair.second.speed);
      preferences.putInt(breakPointStoreDuty.c_str(), pair.second.speed);
      preferences.putInt(breakPointStoreMethanol.c_str(), pair.second.methanol);

    }
    printMap();
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
  cycleOutput(1000, ledPin, outputFlag0);
  cycleOutput(1000, valvePin, outputFlag1);
  cycleOutput(1000, pumpRelayPin, outputFlag2);

};





// server.on("/updateDemands", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
//   DynamicJsonDocument doc(1024);
//   DeserializationError error = deserializeJson(doc, data);
//   if (error) {
//     Serial.print(F("deserializeJson() failed: "));
//     Serial.println(error.f_str());
//     return;
//   }

//   // Expecting a JSON object so check it's the correct type
//   if (!doc.is<JsonObject>()) {
//     Serial.println(F("JSON is not an object"));
//     return;
//   }

//   JsonObject obj = doc.as<JsonObject>();

//   // Do something with the data here. Here's an example:
//   for(JsonPair p : obj) {
//     Serial.println(p.key().c_str());
//     Serial.println(p.value().as<int>());
//     // You could store the values in the ESP32's preferences here. 
//     // Replace "my-app" with your preference namespace and "p.key().c_str()" with your preference key.
//     preferences.begin("my-app", false);
//     preferences.putInt(p.key().c_str(), p.value().as<int>());
//     preferences.end();
//   }

//   request->send(200, "application/json", "{\"message\":\"Successfully updated demands\"}");
// });

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Arduino.h>
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
#define USE_INTRANET
// #define USE_AP
// #define SIMULATOR

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
int sensorInput[] = {pushToInjectPin, flowMeterPin, injectorDutyPin, spareInputPin};

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

struct Values {
    int key_3000 = 0;
    int key_3500 = 0;
    int key_4000 = 0;
    int key_4500 = 0;
    int key_5000 = 0;
    int key_5500 = 0;
    int key_6000 = 0;
    int key_6500 = 0;
    int key_7000 = 0;
};


// PreferencesManager prefMan;


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
    <br>
    <input type="text" id="var1" placeholder="Enter value for Injection Start RPM">
    <button onclick="updateStartRPM()">Update Start RPM</button>
    <br>
    <input type="text" id="var2" placeholder="Enter value for Injection End RPM">
    <button onclick="updateEndRPM()">Update End RPM</button>
  </div>
  <script>

    // Define the number of outputs
    var numOutputs = 4;

    window.onload = function () {
      let xhr = new XMLHttpRequest();
      xhr.open("GET", "/getOutputStates", true);
      xhr.onload = function () {
        if (xhr.status == 200) {
          let relayStates = JSON.parse(xhr.responseText);
          for (let i = 0; i < numOutputs; i++) {
            let checkbox = document.getElementById(i);
            checkbox.checked = relayStates[i.toString()] === "HIGH";
          }
        }
      };
      xhr.send();
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

  server.on("/updateStartRPM", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  if (request->hasParam("startRPM")) {
    // open up preferences
    preferences.begin("my-app", false);

    // get the value from the web request and store
    int injectionStartRPM; 
    injectionStartRPM = request->getParam("startRPM")->value().toInt();
    Serial.println("Injection Start RPM is: " + String(injectionStartRPM));
    preferences.putInt("startRPM", injectionStartRPM);

    // retreive the int for testing purposes
    Serial.println(preferences.getInt("startRPM", 0));

    // close preferences
    preferences.end();

  }
  request->send(200, "text/plain", String(injectionStartRPM)); });

  server.on("/updateEndRPM", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  if (request->hasParam("endRPM")) {
    // open preferences
    preferences.begin("my-app", false);
    
    // get the vale from the web request and store
    int injectionEndRPM;
    injectionEndRPM = request->getParam("endRPM")->value().toInt();
    Serial.println("Injection End RPM is: " + String(injectionEndRPM));
    preferences.putInt("endRPM", injectionEndRPM);

    // retrieve for testing purposes
    Serial.println(Serial.println(preferences.getInt("endRPM", 0)));
  }
  request->send(200, "text/plain", String(injectionEndRPM)); });


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
    // Serial.println("start" + String(prefMan.getPreference("startRPM")));
    // doc["startRPM"] = prefMan.getPreference("startRPM");
    // doc["endRPM"] = prefMan.getPreference("endRPM");
    doc["startRPM"] = preferences.getUInt("start", 0);
    Serial.println(preferences.getString("start"));
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

// Start server
server.begin();

}

template <typename T>
void printType(const T& var) {
  if (std::is_same<T, int>::value)
    Serial.println("int");
  else if (std::is_same<T, float>::value)
    Serial.println("float");
  else if (std::is_same<T, String>::value)
    Serial.println("String");
  else if (std::is_same<T, unsigned int>::value)
    Serial.println("unsigned int");
  else
    Serial.println("Unknown type");
}

void loop()
{
  cycleOutput(1000, ledPin, outputFlag0);
  cycleOutput(1000, valvePin, outputFlag1);
  cycleOutput(1000, pumpRelayPin, outputFlag2);

}

// void printWifiStatus()
// {

//   // print the SSID of the network you're attached to:
//   Serial.print("SSID: ");
//   Serial.println(WiFi.SSID());

//   // print your WiFi shield's IP address:
//   ip = WiFi.localIP();
//   Serial.print("IP Address: ");
//   Serial.println(ip);

//   // print the received signal strength:
//   long rssi = WiFi.RSSI();
//   Serial.print("signal strength (RSSI):");
//   Serial.print(rssi);
//   Serial.println(" dBm");
//   // print where to go in a browser:
//   Serial.print("Open http://");
//   Serial.println(ip);
// }
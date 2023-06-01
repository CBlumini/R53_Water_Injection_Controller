#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Arduino.h>
#include <secrets.h>
#include <ArduinoJson.h>
// #include <web_page.h>

// here you post web pages to your homes intranet which will make page debugging easier
// as you just need to refresh the browser as opposed to reconnection to the web server
#define USE_INTRANET

// once  you are read to go live these settings are what you client will connect to
#define AP_SSID "Water Inject"
#define AP_PASS NULL

#define NORMALLY_OPEN true

// Networking Setup
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;
// variable for the IP reported when you connect to your homes intranet (during debug mode)
IPAddress actualIP;

AsyncWebServer server(80);


// Outputs
int ledPin = 2;
int valvePin = 25;
int pumpRelayPin = 26;
int spareOutputPin = 27;
// Inputs
int pushToInjectPin = 34;
int flowMeterPin = 35;
int injectorDutyPin = 36;
int spareInputPin = 39;

int actuatorOutputs[] = {ledPin, valvePin, pumpRelayPin, spareOutputPin};
int sensorInput[] = {pushToInjectPin, flowMeterPin, injectorDutyPin, spareInputPin};
// int NUM_OUTPUTS = 3;

const char *RELAY_REF = "relay";
const char *STATE_REF = "state";
int injectionStartRPM = 0;
int injectionEndRPM = 0;
float scalingFactor = 0.0;

// html code
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
    h2 {font-size: 2.5rem;}
    p {font-size: 2.5rem;}
    body {
      max-width: 600px;
      margin: 0px auto;
      padding-bottom: 25px;
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
    input:checked + .slider {
      background-color: #2196F3;
    }
    input:checked + .slider:before {
      -webkit-transform: translateX(26px);
      -ms-transform: translateX(26px);
      transform: translateX(26px);
    }
    .flex-container{
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
    }
  </style>
</head>
<body>
  <h3>Blumini Injects Monitor and Test Page</h3>
    <div class="flex-container">
        <div id="buttons"></div>
        <P>
        <!-- %BUTTONPLACEHOLDER% -->
        <!-- <h4>Pump Test</h4>
        <label class="switch">
            <input type="checkbox" onchange="toggleCheckbox(this)" id=0
        </label> -->
        </P>
        <p>
        <input type="text" id="var1" placeholder="Enter value for Injection Start RPM">
        <button onclick="updateStartRPM()">Update Start RPM</button>
        </p>
        <p>
        <input type="text" id="var2" placeholder="Enter value for Injection End RPM">
        <button onclick="updateEndRPM()">Update End RPM</button>
        </p>
    </div>
  <script>
    window.onload = function() {
      let xhr = new XMLHttpRequest();
      xhr.open("GET", "/getRelayStates", true);
      xhr.onload = function() {
        if (xhr.status == 200) {
          let relayStates = JSON.parse(xhr.responseText);
          for (let i = 0; i < numOutputs; i++) {
           let checkbox = document.getElementById(i);
            checkbox.checked = relayStates[i.toString()] == HIGH;
          }
        }
      };
      xhr.send();
    }

    // Define the number of outputs
    var numOutputs = 3;

    // Get the buttons div
    var buttonsDiv = document.getElementById('buttons');

    // Create a checkbox and label for each output
    for (var i = 0; i < numOutputs; i++) {
      // Create checkbox
      var checkbox = document.createElement('input');
      checkbox.type = 'checkbox';
      checkbox.id = i;
      checkbox.onchange = function() { toggleCheckbox(this); };

      // Create label
      var label = document.createElement('label');
      label.htmlFor = i;
      label.textContent = ' Relay #' + (i+1);

      // Append checkbox and label to div
      buttonsDiv.appendChild(checkbox);
      buttonsDiv.appendChild(label);
      buttonsDiv.appendChild(document.createElement('br'));
    }

    function toggleCheckbox(element) {
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
      if (!Number.isInteger(Number(startRPM))){
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
      if (!Number.isInteger(Number(endRPM))){
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

// I think I got this code from the wifi example
void printWifiStatus()
{

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("Open http://");
  Serial.println(ip);
}

// end of code

String getOutputStatus(int numRelay)
{
  if (NORMALLY_OPEN)
  {
    if (digitalRead(actuatorOutputs[numRelay - 1]))
    {
      return "";
    }
    else
    {
      return "checked";
    }
  }
  else
  {
    if (digitalRead(actuatorOutputs[numRelay - 1]))
    {
      return "checked";
    }
    else
    {
      return "";
    }
  }
  return "";
}


///////////////// MAIN CODE ///////////////////
void setup()
{
//WIFI SETUP
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
#ifndef USE_INTRANET
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  Actual_IP = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(Actual_IP);
#endif
  ////////////// END WIFI SETUP//////////
#pragma endregion

  Serial.begin(115200);

  // Set up the output pins
  pinMode(ledPin, OUTPUT);
  ledcSetup(0, 100, 8);
  ledcAttachPin(valvePin, 0);
  pinMode(pumpRelayPin, OUTPUT);
  pinMode(spareOutputPin, OUTPUT);

  // Set up the input pins

  ///////////////SETUO ROUTES////////////
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  // Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String relayId;
    String idParam;
    String relayState;
    String stateParam;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(RELAY_REF) & request->hasParam(STATE_REF)) {
      relayId = request->getParam(RELAY_REF)->value();
      idParam = RELAY_REF;
      relayState = request->getParam(STATE_REF)->value();
      stateParam = STATE_REF;
      if(NORMALLY_OPEN){
        Serial.print("NO ");
        digitalWrite(actuatorOutputs[relayId.toInt()-1], !relayState.toInt());
      }
      else{
        Serial.print("NC ");
        digitalWrite(actuatorOutputs[relayId.toInt()-1], relayState.toInt());
      }
    }
    else {
      relayId = "No message sent";
    }
    Serial.println(relayId + relayState);
    request->send(200, "text/plain", "OK"); });


  server.on("/updateStartRPM", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  if (request->hasParam("startRPM")) {
    injectionStartRPM = request->getParam("startRPM")->value().toInt();
    Serial.println("Injection Start RPM is: " + String(injectionStartRPM));
  }

  request->send(200, "text/plain", String(injectionStartRPM)); 
  });

  server.on("/updateEndRPM", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  if (request->hasParam("endRPM")) {
    injectionEndRPM = request->getParam("endRPM")->value().toInt();
    Serial.println("Injection End RPM is: " + String(injectionEndRPM));
  }

  request->send(200, "text/plain", String(injectionEndRPM)); 
  });

  server.on("/getRelayStates", HTTP_GET, [](AsyncWebServerRequest *request){
    DynamicJsonDocument doc(1024);
    for (int i = 0; i < 4; i++)
    {
      doc[String(i)] = digitalRead(actuatorOutputs[i]);
    }
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Start server
  server.begin();
}

void loop()
{
}
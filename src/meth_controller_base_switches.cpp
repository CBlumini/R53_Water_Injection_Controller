#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Arduino.h>
#include <secrets.h>
// #include <web_page.h>

// here you post web pages to your homes intranet which will make page debugging easier
// as you just need to refresh the browser as opposed to reconnection to the web server
#define USE_INTRANET

// replace this with your homes intranet connect parameters
// moved this to a special file

// once  you are read to go live these settings are what you client will connect to
#define AP_SSID "Water Inject"
#define AP_PASS NULL

#define NORMALLY_OPEN  true

// Netwroking Setup
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;
// variable for the IP reported when you connect to your homes intranet (during debug mode)
IPAddress Actual_IP;

AsyncWebServer server(80);

int actuatorOutputs[] = {2, 26};
// int NUM_OUTPUTS = sizeof(relayGPIOs)/sizeof(relayGPIOs[0]);
int NUM_OUTPUTS = 2;

const char* RELAY_REF = "relay";  
const char* STATE_REF = "state";
int injectionStartRPM = 0;
int injectionEndRPM = 0;
float scalingFactor = 0.0;

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
        <P>
        %BUTTONPLACEHOLDER%
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
      if (!Number.isInteger(Number(startRPM))){
        alert("Must be a whole number");
        return;
      }
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/updateStartRPM?startRPM=" + startRPM);
      
      xhr.onerror = function() {
        alert("Request failed.");
      };
      xhr.send();
    }
    function updateEndRPM() {
      let endRPM = document.getElementById('var1').value;
      if (!Number.isInteger(Number(endRPM))){
        alert("Must be a whole number");
        return;
      }
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/updateEndtRPM?endRPM=" + endRPM);
      
      xhr.onerror = function() {
        alert("Request failed.");
      };
      xhr.send();
    }
  </script>
</body>
</html>
)rawliteral";

// I think I got this code from the wifi example
void printWifiStatus() {

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

String getOutputStatus(int numRelay){
  if(NORMALLY_OPEN){
    if(digitalRead(actuatorOutputs[numRelay-1])){
      return "";
    }
    else {
      return "checked";
    }
  }
  else {
    if(digitalRead(actuatorOutputs[numRelay-1])){
      return "checked";
    }
    else {
      return "";
    }
  }
  return "";
}


// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    String pumpState = getOutputStatus(0);
    buttons = "<h4>Pump Test</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id = 0 "+ pumpState +"><span class=\"slider\"></span></label>";
    String valveState = getOutputStatus(1);
    buttons += "<h4>Valve Test</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id = 1 "+ valveState +"><span class=\"slider\"></span></label>";
    

    return buttons;
  }
  return String();
}

///////////////// MAIN CODE ///////////////////
void setup(){
  Serial.begin(115200);

  // Set up the pinst
  for(int i=1; i<=NUM_OUTPUTS; i++){
    pinMode(actuatorOutputs[i-1], OUTPUT);
    if(NORMALLY_OPEN){
      digitalWrite(actuatorOutputs[i-1], HIGH);
    }
    else{
      digitalWrite(actuatorOutputs[i-1], LOW);
    }
  }
  

#ifdef USE_INTRANET
  WiFi.begin(LOCAL_SSID, LOCAL_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  Actual_IP = WiFi.localIP();
#endif

  // if you don't have #define USE_INTRANET, use AP Mode
#ifndef USE_INTRANET
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  Actual_IP = WiFi.softAPIP();
  Serial.print("IP address: "); Serial.println(Actual_IP);
#endif
////////////// END WIFI SETUP//////////


///////////////SETUO ROUTES////////////
// Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
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
    request->send(200, "text/plain", "OK");
  });

  server.on("/updateVariables", HTTP_GET, [] (AsyncWebServerRequest *request) {
  if (request->hasParam("var1") && request->hasParam("var2")) {
    injectionStartRPM = request->getParam("var1")->value().toInt();
    injectionEndRPM = request->getParam("var2")->value().toInt();
    
    Serial.println("Injection start RPM is: " + String(injectionStartRPM));
    Serial.println("Injection start RPM is: " + String(injectionEndRPM));
  }

  request->send(200, "text/plain", "OK");
});

  // Start server
  server.begin();
}

void loop() {

}
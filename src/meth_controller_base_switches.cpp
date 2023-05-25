#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Arduino.h>
#include <secrets.h>

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

const char* PARAM_INPUT_1 = "relay";  
const char* PARAM_INPUT_2 = "state";

// // Access point credentials
// const char* ssid = "ESP32_AP";
// const char* password = NULL;


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h3>Blumini Injects Monitor and Test Page</h3>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
  xhr.send();
}</script>
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
    // for(int i=1; i<=NUM_OUTPUTS; i++){
    //   String relayStateValue = getOutputStatus(i);
    //   buttons+= "<h4>Relay #" + String(i) + " - GPIO " + actuatorOutputs[i-1] + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" "+ relayStateValue +"><span class=\"slider\"></span></label>";
    // }
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
  
  /////////////// SETUP WIFI
  // Serial.println("Setting up Wi-Fi access point...");
  // WiFi.mode(WIFI_AP);
  // bool result = WiFi.softAP(ssid, password);

  // if (result == true) {
  //   Serial.println("Access point successfully created!");
  // } else {
  //   Serial.println("Failed to create access point. Check your SSID and password.");
  // }

  // delay(1000); // Give the ESP32 some time to create the network

  // // Print ESP32 Local IP Address
  // IPAddress IP = WiFi.softAPIP();
  // Serial.print("AP IP address: ");
  // Serial.println(IP);

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

  ////////////// END WIFI SETUP
  
// Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      if(NORMALLY_OPEN){
        Serial.print("NO ");
        digitalWrite(actuatorOutputs[inputMessage.toInt()-1], !inputMessage2.toInt());
      }
      else{
        Serial.print("NC ");
        digitalWrite(actuatorOutputs[inputMessage.toInt()-1], inputMessage2.toInt());
      }
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage + inputMessage2);
    request->send(200, "text/plain", "OK");
  });
  // Start server
  server.begin();
}

void loop() {

}
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
 


int currentTime = 12332; //Current time
int moistureThreshold = 32; //Threshold for when the watering should happen
int currentMoistureLevel = 999; //Current moisture level in the soil
int cooldownMinutes = 21; //Cooldown before the system will consider watering again
int wateringTimeSeconds = 2; //amount of the water is sent from the pump to the plant
int lastWatering = 4222; //the amount of time passed since last watering



const char* _wifiName = "FTTH_WL1722";
const char* _wifiPassword = "meawhivRyar9";
ESP8266WebServer server(80);
 
// Serving Hello world
void getSystemValues() 
{
    DynamicJsonDocument doc(1024);
    doc["moistureThreshold"] = moistureThreshold;
    doc["currentMoistureLevel"] = currentMoistureLevel;
    doc["cooldownMinutes"] = cooldownMinutes;
    doc["wateringTimeSeconds"] = wateringTimeSeconds;
    doc["lastWateringInHours"] = String((currentTime - lastWatering) * 2.7777777777778E-7);

    server.send(200, "text/json", doc.as<String>());
}

void setWateringTimeSeconds()
{

  String arg = "wateringTimeSeconds";

  if(!server.hasArg(arg))
  {
    server.send(400, "text/json", "Missing argument: " + arg);
    return;
  }

  int receivedwateringTimeSeconds = server.arg(arg).toInt();

  if(receivedwateringTimeSeconds == 0)
  {
    server.send(400, "text/json", "Value could not be converted to an integer");
    return;
  }

  int oldwateringTimeSeconds = wateringTimeSeconds;
  wateringTimeSeconds = receivedwateringTimeSeconds;

  server.send(200, "text(json", "MoistureThreshold changed from " + String(oldwateringTimeSeconds) + " to " + String(wateringTimeSeconds));

}


void setCooldownMinutes()
{

  String arg = "cooldownMinutes";

  if(!server.hasArg(arg))
  {
    server.send(400, "text/json", "Missing argument: " + arg);
    return;
  }

  int receivedcooldownMinutes = server.arg(arg).toInt();

  if(receivedcooldownMinutes == 0)
  {
    server.send(400, "text/json", "Value could not be converted to an integer");
    return;
  }

  int oldcooldownMinutes = cooldownMinutes;
  cooldownMinutes = receivedcooldownMinutes;

  server.send(200, "text(json", "MoistureThreshold changed from " + String(oldcooldownMinutes) + " to " + String(cooldownMinutes));

}
 
void setMoistureThreshold()
{

  String arg = "moistureThreshold";

  if(!server.hasArg(arg))
  {
    server.send(400, "text/json", "Missing argument: " + arg);
    return;
  }

  int receivedMoistureThreshold = server.arg(arg).toInt();

  if(receivedMoistureThreshold == 0)
  {
    server.send(400, "text/json", "Value could not be converted to an integer");
    return;
  }

  int oldMoistureThreshold = moistureThreshold;
  moistureThreshold = receivedMoistureThreshold;

  server.send(200, "text(json", "MoistureThreshold changed from " + String(oldMoistureThreshold) + " to " + String(moistureThreshold));

}

void healthCheck()
{
  server.send(200, "json/text");
}

// Define routing
void restServerRouting() 
{
    server.on("/", HTTP_GET, []() 
    {
        server.send(200, F("text/html"),
            F("Welcome to the REST Web Server"));
    });

    server.on(F("/health-check"), HTTP_GET, healthCheck);
    server.on(F("/get-watering-system-values"), HTTP_GET, getSystemValues); 
    server.on(F("/set-watering-time-seconds"), HTTP_PUT, setWateringTimeSeconds);
    server.on(F("/set-cooldown-minutes"), HTTP_PUT, setCooldownMinutes);
    server.on(F("/set-moisture-threshold"), HTTP_PUT, setMoistureThreshold);
}

// Manage not found URL
void handleNotFound() 
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) 
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void connectToWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(_wifiName, _wifiPassword);
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(_wifiName);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
 
  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}
 


// -----------------------------------------------

#define pin D2

void setup(void) 
{
  Serial.begin(9600);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  // connectToWiFi();
}
 
void loop(void) {
  // server.handleClient();

  // digitalWrite(pin, HIGH);
  // delay(500);
  // digitalWrite(pin, HIGH);
  // delay(500);

}








 

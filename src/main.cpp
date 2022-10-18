#include "Arduino.h"
#include "WaterPumpService.h"
#include "SoilSensorService.h"
#include "MathService.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
 
 //GPIO
#define soilSensorReadGPIO A0
#define soilSensorActivateGPIO D5
#define waterPumpGPIO D7


//Server function definitions
void getSystemValues();
void setWateringTimeSeconds();
void setMoistureThreshold();
void healthCheck();
void restServerRouting();
void handleNotFound();
void connectToWiFi();

//Wifi variables and objects
const char* _wifiName = "FTTH_WL1722";
const char* _wifiPassword = "meawhivRyar9";
ESP8266WebServer server(80);

//Core system variables
long currentTime = millis(); //Current time
int minDrynessAllowed = 600; //Threshold for when the watering should happen
int wateringTimeSeconds = 5; //amount of the water is sent from the pump to the plant
int lastWateringHours = 0; //the amount of time passed since last watering
byte soilReadingFrequencyMinutes = 60; //How often a soilreading should happen
long lastSoilReading = 0; //holds last millis() a reading was done
int numberOfSoilReadings = 1000; //number of soilreading done - avg is calculated
double averageSoilReading = 0; //calculated soilreading
bool soilSensorOnline = false;

//Custom classes
WaterPumpService waterPumpService;
SoilSensorService soilSensorService;
MathService mathService;


void setup(void) 
{
  Serial.begin(9600);
  // connectToWiFi();
  pinMode(waterPumpGPIO, OUTPUT);
  pinMode(soilSensorReadGPIO, INPUT);
  pinMode(soilSensorActivateGPIO, OUTPUT);
}
 
void loop(void) 
{
  server.handleClient();

  currentTime = millis();

  if(currentTime - lastSoilReading < mathService.ConvertMinutesToMillis(soilReadingFrequencyMinutes))
  {
    return;
  }
  
  lastSoilReading = currentTime;

  averageSoilReading = 0;

  soilSensorService.ActivateSoilSensor(soilSensorActivateGPIO);
  soilSensorOnline = true;

  for(int i = 0; i < numberOfSoilReadings; i++)
  {
    averageSoilReading = averageSoilReading + soilSensorService.GetSensorReading(soilSensorReadGPIO);
  }

  soilSensorService.DisableSoilSensor(soilSensorActivateGPIO);
  soilSensorOnline = false;

  averageSoilReading = averageSoilReading / numberOfSoilReadings;

  if(averageSoilReading >= minDrynessAllowed && !soilSensorOnline)
  {
    return;
  }

  waterPumpService.StartWaterPump(waterPumpGPIO);

  long waterPumpTime = millis();

  while(waterPumpTime - currentTime < mathService.ConvertSecondsToMillis(wateringTimeSeconds))
  {
    waterPumpTime = millis();
    delay(15); 
  }

  waterPumpService.StopWaterPump(waterPumpGPIO);

  lastWateringHours = mathService.ConvertMillisToHours(millis());
    
}




 


//------------ API ------------

void getSystemValues() 
{
    DynamicJsonDocument doc(1024);
    doc["minDrynessAllowed"] = minDrynessAllowed;
    doc["averageSoilReading"] = averageSoilReading;
    doc["wateringTimeSeconds"] = wateringTimeSeconds;
    doc["lastWateringInHours"] = String((currentTime - lastWateringHours) * 2.7777777777778E-7);

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

  server.send(200, "text(json", "wateringTimeSeconds changed from " + String(oldwateringTimeSeconds) + " to " + String(wateringTimeSeconds));

}

 
void setMinDrynessAllowed()
{

  String arg = "minDrynessAllowed";

  if(!server.hasArg(arg))
  {
    server.send(400, "text/json", "Missing argument: " + arg);
    return;
  }

  int receivedMinDrynessAllowed = server.arg(arg).toInt();

  if(receivedMinDrynessAllowed == 0)
  {
    server.send(400, "text/json", "Value could not be converted to an integer");
    return;
  }

  int oldMinDrynessAllowed = minDrynessAllowed;
  minDrynessAllowed = receivedMinDrynessAllowed;

  server.send(200, "text(json", "Minimum dryness allowed changed from " + String(oldMinDrynessAllowed) + " to " + String(minDrynessAllowed));

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
    server.on(F("/set-minimum-dryness-allowed"), HTTP_PUT, setMoistureThreshold);
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
 

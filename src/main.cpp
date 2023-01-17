#include "Arduino.h"
#include "WaterPumpService.h"
#include "SoilSensorService.h"
#include "MathService.h"
#include "UrlEncoderDecoder.h"
#include <ESP8266WiFi.h>
#include <ESP8266HttpClient.h>
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
void setMinDrynessAllowed();
void daysBeforeNextReset();
void healthCheck();
void restServerRouting();
void SendSMS(String message);
void handleNotFound();
void connectToWiFi();
void requestWatering();
void RunWateringCycle();
void setSoilReadingFrequencyMinutes();
void setSoilReadingFrequencyMinutes();
void getCurrentSoilReading();

//Wifi variables and objects
ESP8266WebServer server(80);
HTTPClient client;
WiFiClientSecure wifiClientSecure;

const char* _wifiName = "";
const char* _wifiPassword = "";
const String toSMS = "+";
const String fromSMS = "+";
const String AccountIdSMS = "";
const String token = "==";
const char fingerprint[] = "B7D";
const char* host = "";
const int   httpsPort = 0;
const String ResetSystemMessage = "Selfwatering system: Reset Required";
const String RefillWaterMessage = "Selfwatering system: Refill water";


//Core system variables
unsigned long currentTimeMillis = millis(); //Current time
int drynessAllowed = 400; //Threshold for when the watering should happen
int wateringTimeSeconds = 5; //amount of the water is sent from the pump to the plant
int lastWateringMillis = 0; //the amount of time passed since last watering
byte soilReadingFrequencyMinutes = 60; //How often a soilreading should happen
unsigned long lastSoilReadingMillis = 0; //holds last millis() a reading was done
int numberOfSoilReadings = 1000; //number of soilreading done - avg is calculated
double averageSoilReading = 0; //calculated soilreading
byte daysLeftBeforeReset = 1; //Reset system when currentTime is 1 day from reaching max value of unsigned long
bool wateringAutomationEnabled = true;
double averageSoilReadingBeforeLastWatering;
bool averageSoilReadingBeforeLastWateringSat = false;
bool notified = false;

//Custom classes
WaterPumpService waterPumpService;
SoilSensorService soilSensorService;
MathService mathService;
UrlEncoderDecoderService urlEncoderDecoderService;


void setup(void) 
{
  Serial.begin(9600);
  // connectToWiFi();
  pinMode(waterPumpGPIO, OUTPUT);
  pinMode(soilSensorReadGPIO, INPUT);
  pinMode(soilSensorActivateGPIO, OUTPUT);
  averageSoilReadingBeforeLastWatering = 0;
}
 
void loop(void) 
{
  
  // server.handleClient();

  // currentTimeMillis = millis();

  // if(mathService.ConvertMillisToDays(ULONG_MAX - currentTimeMillis) <= daysLeftBeforeReset)
  // {
  //   SendSMS("Selfwatering system: Reset Required");
  // }

  // if(!wateringAutomationEnabled)
  // {
  //   return;
  // }

  // if((currentTimeMillis - lastSoilReadingMillis < mathService.ConvertMinutesToMillis(soilReadingFrequencyMinutes)))
  // {
  //   return;
  // }
  
  // lastSoilReadingMillis = currentTimeMillis;

  // averageSoilReading = 0;

  // soilSensorService.ActivateSoilSensor(soilSensorActivateGPIO);

  // for(int i = 0; i < numberOfSoilReadings; i++)
  // {
  //   averageSoilReading = averageSoilReading + soilSensorService.GetSensorReading(soilSensorReadGPIO);
  // }

  // soilSensorService.DisableSoilSensor(soilSensorActivateGPIO);

  // averageSoilReading = averageSoilReading / numberOfSoilReadings;

  // if(averageSoilReading <= drynessAllowed)
  // {
  //   return;
  // }

  averageSoilReading = 1;

  if(averageSoilReadingBeforeLastWateringSat == false)
  {
    averageSoilReadingBeforeLastWateringSat = true;
    Serial.println("1");
    averageSoilReadingBeforeLastWatering = averageSoilReading;
  }
  else if((averageSoilReading > averageSoilReadingBeforeLastWatering) && (!notified))
  {
    Serial.println("2");
    // SendSMS(RefillWaterMessage);
    notified = true;
    averageSoilReadingBeforeLastWatering = averageSoilReading;
  }
  else if(averageSoilReading < averageSoilReadingBeforeLastWatering)
  {
    notified = false;
    Serial.println("3");
  }

  // RunWateringCycle();
  delay(3000);
    
}

void RunWateringCycle()
{

  waterPumpService.StartWaterPump(waterPumpGPIO);

  long waterPumpTime = millis();

  while(waterPumpTime - currentTimeMillis < mathService.ConvertSecondsToMillis(wateringTimeSeconds))
  {
    waterPumpTime = millis();
    delay(15); //Avoid watchdog in ESP8266 12E
  }

  waterPumpService.StopWaterPump(waterPumpGPIO);

  lastWateringMillis = millis();
}


 


//------------ API ------------

void SendSMS(String message)
{
  String data = "To=" + urlEncoderDecoderService.urlencode(toSMS) + "&From=" + urlEncoderDecoderService.urlencode(fromSMS) + "&Body=" + urlEncoderDecoderService.urlencode(message);

  client.POST(data);
}

void getSystemValues() 
{
    DynamicJsonDocument doc(1024);
    doc["DrynessAllowedBeforeWatering"] = drynessAllowed;
    doc["LastSoilReadingAverageValue"] = averageSoilReading;
    doc["WateringTimeSeconds"] = wateringTimeSeconds;
    doc["MinutesBetweenSoilReadings"] = soilReadingFrequencyMinutes;
    doc["MinutesAgoSinceLastSoilReading"] = String(mathService.ConvertMillisToMinutes(currentTimeMillis - lastSoilReadingMillis));
    doc["HoursAgoLastWateringCycleWasDone"] = String(mathService.ConvertMillisToHours(currentTimeMillis - lastWateringMillis));
    doc["WateringAutomationEnabled"] = wateringAutomationEnabled;

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

  if(receivedwateringTimeSeconds > 10)
  {
    server.send(400, "text/json", "Value cannot be larger than 10");
    return;
  }

  int oldwateringTimeSeconds = wateringTimeSeconds;
  wateringTimeSeconds = receivedwateringTimeSeconds;

  server.send(200, "text/json", "wateringTimeSeconds changed from " + String(oldwateringTimeSeconds) + " to " + String(wateringTimeSeconds));

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

  if(receivedMinDrynessAllowed > 435)
  {
    server.send(400, "text/json", "Max dryness allowed is 410. When measuring dry soil the value was 435 - 438");
    return;
  }

  if(receivedMinDrynessAllowed < 350)
  {
    server.send(400, "text/json", "Min dryness allowed is 350. When measuring newly watered soil the value was 285 - 287");
    return;
  }

  int oldMinDrynessAllowed = drynessAllowed;
  drynessAllowed = receivedMinDrynessAllowed;

  server.send(200, "text/json", "Minimum dryness allowed changed from " + String(oldMinDrynessAllowed) + " to " + String(drynessAllowed));

}

void setSoilReadingFrequencyMinutes()
{

  String arg = "soilReadingFrequencyMinutes";

  if(!server.hasArg(arg))
  {
    server.send(400, "text/json", "Missing argument: " + arg);
    return;
  }

  int receivedSoilReadingFrequencyMinutes = server.arg(arg).toInt();

  if(receivedSoilReadingFrequencyMinutes == 0)
  {
    server.send(400, "text/json", "Value could not be converted to an integer");
    return;
  }

  if(receivedSoilReadingFrequencyMinutes > 120)
  {
    server.send(400, "text/json", "Value cannot be larger than 120");
    return;
  }

  int oldSoilReadingFrequencyMinutes = soilReadingFrequencyMinutes;
  soilReadingFrequencyMinutes = receivedSoilReadingFrequencyMinutes;

  server.send(200, "text/json", "Soil reading frequency changed from " + String(oldSoilReadingFrequencyMinutes) + " to " + String(soilReadingFrequencyMinutes));

}


void toggleWateringAutomationEnabled()
{

  if(wateringAutomationEnabled)
  {
    wateringAutomationEnabled = false;
    server.send(200, "text/json", "Watering system DISABLED");
  }
  else
  {
    wateringAutomationEnabled = true;
    server.send(200, "text/json", "Watering system ENABLED");
  }

  

}

void daysBeforeNextReset()
{

  double daysLeft = mathService.ConvertMillisToDays(ULONG_MAX - currentTimeMillis) - daysLeftBeforeReset;

  server.send(200, "text/json", "System will reset in: " + String(daysLeft));

}

void requestWatering()
{
  RunWateringCycle();

  lastSoilReadingMillis = millis(); //Allow water to settle before next reading is done

  server.send(200, "text/json", "Watering cycle completed");

}

void getCurrentSoilReading()
{
  double soilReading = 0;

  soilSensorService.ActivateSoilSensor(soilSensorActivateGPIO);

  for(int i = 0; i < numberOfSoilReadings; i++)
  {
    soilReading = soilReading + soilSensorService.GetSensorReading(soilSensorReadGPIO);
  }

  soilSensorService.DisableSoilSensor(soilSensorActivateGPIO);

  soilReading = soilReading / numberOfSoilReadings;

  server.send(200, "text/json", "Soilreading: " + String(soilReading));
}

void healthCheck()
{
  server.send(200, "text/json");
}

// Define routing
void restServerRouting() 
{
    server.on(F("/run-watering-cycle"), HTTP_POST, requestWatering);
    server.on(F("/health-check"), HTTP_GET, healthCheck);
    server.on(F("/get-watering-system-values"), HTTP_GET, getSystemValues); 
    server.on(F("/get-days-before-system-reset"), HTTP_GET, daysBeforeNextReset);
    server.on(F("/get-current-soil-reading"), HTTP_GET, getCurrentSoilReading);
    server.on(F("/set-watering-time-seconds"), HTTP_PUT, setWateringTimeSeconds);
    server.on(F("/set-minimum-dryness-allowed"), HTTP_PUT, setMinDrynessAllowed);
    server.on(F("/set-soil-reading-frequency"), HTTP_PUT, setSoilReadingFrequencyMinutes);
    server.on(F("/toggle-watering-automation"), HTTP_PUT, toggleWateringAutomationEnabled);
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
  Serial.println("Setting up client...");

  wifiClientSecure.setFingerprint(fingerprint);
  wifiClientSecure.connect(host, httpsPort);
        
  client.begin(wifiClientSecure, "https://" + host + "/2010-04-01/Accounts/" + AccountIdSMS + "/Messages.json");
  client.addHeader("Content-Type", "application/x-www-form-urlencoded");
  client.addHeader("Authorization", token);
  
}

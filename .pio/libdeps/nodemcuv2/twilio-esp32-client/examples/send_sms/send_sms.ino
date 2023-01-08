#include "twilio.hpp"

// Set these - but DON'T push them to GitHub!
static const char *ssid = "";
static const char *password = "";

// Values from Twilio (find them on the dashboard)
static const char *account_sid = "";
static const char *auth_token = "";
// Phone number should start with "+<countrycode>"
static const char *from_number = "";

// You choose!
// Phone number should start with "+<countrycode>"
static const char *to_number = "";
static const char *message = "Sent from my ESP32";

Twilio *twilio;

void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to WiFi network ;");
  Serial.print(ssid);
  Serial.println("'...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay(500);
  }
  Serial.println("Connected!");

  twilio = new Twilio(account_sid, auth_token);

  delay(1000);
  String response;
  bool success = twilio->send_message(to_number, from_number, message, response);
  if (success) {
    Serial.println("Sent message successfully!");
  } else {
    Serial.println(response);
  }
}

void loop() {
}

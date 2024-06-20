// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


// ===========================
//      WiFi credentials
// ===========================
const char* ssid = "No On3";
const char* password = "mm01147405183";

// ===========================
//        MQTT DETAILS
// ===========================
const char *mqtt_server = "b37.mqtt.one";   //https://test.mosquitto.org/
const char *MQTT_USER = "18ciqt4398"    ;
const char *MQTT_PASSWORD = "259bhmopuv";
/* MQTT TOPICS */
const char *mqtt_topic_1 = "18ciqt4398/greenhouse";
#define MAX_JSON_STRING_LENGTH 100

void callback(char* topic, byte* payload, unsigned int length);
WiFiClient espClient;
PubSubClient client(espClient);

#define DHTPIN 4     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 3 (on the right) of the sensor to GROUND (if your sensor has 3 pins)
// Connect pin 4 (on the right) of the sensor to GROUND and leave the pin 3 EMPTY (if your sensor has 4 pins)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

void sendGreenhouseDataMQTT(int humidity, int temprature, int soil)
{
  
  StaticJsonDocument<MAX_JSON_STRING_LENGTH> doc;

  doc["greenhouseId"] = "cf5b11";
  
  doc["temp"] = temprature;

  doc["humidity"] = humidity;

  doc["soil"] = soil;

  //Serial.println("\nJSON to be sent to Server:");
  //object.prettyPrintTo(Serial);

  char jsonChar[MAX_JSON_STRING_LENGTH];
  serializeJson(doc, jsonChar);
  Serial.println(jsonChar);

  client.publish(mqtt_topic_1, jsonChar);
  //client.publish("18ciqt4398/robot", "@db5f040N123400;");

}


void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    if (client.connect("", MQTT_USER, MQTT_PASSWORD)) 
    {
      /* CONNECT OT MQTT CHANNEL */
      client.subscribe(mqtt_topic_1);
    } 
    else 
    {
      delay(5000);
    }
  }
}




void callback(char* topic, byte* payload, unsigned int length)
{

  // Validate packet length
  
  

}


void setup_wifi() 
{
  delay(10);
  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  
  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  delay(10000);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  int humidityReading = dht.readHumidity();
  // Read temperature as Celsius (the default)
  int tempratureReading = dht.readTemperature();
  // Read soil sensor
  int soilReading = analogRead(A0);
  soilReading = soilReading / 10.24;

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidityReading) || isnan(tempratureReading) || isnan(soilReading)) {
    //Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  sendGreenhouseDataMQTT(humidityReading, tempratureReading, soilReading);
  
}

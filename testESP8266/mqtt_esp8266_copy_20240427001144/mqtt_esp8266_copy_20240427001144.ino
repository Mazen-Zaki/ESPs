
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* WIFI DETAILS */
const char* ssid = "No On3";
const char* password = "mm01147405183";

/* MQTT DETAILS */
const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* MQTT_USER = "18ciqt4398"    ;
const char* MQTT_PASSWORD = "259bhmopuv";

/* MQTT TOPICS */
const char *mqtt_topic_1 = "18ciqt4398/robot";          // to get data from web or AI
const char *mqtt_topic_2 = "18ciqt4398/robotAi";        // to send an error or ACK
const char *mqtt_topic_3 = "mqtt-topic-3";

/* Constants for packet structure */
const char DEVICE_ID_LENGTH = 6   ;
const char PACKET_SIZE = 16       ;
const char START_GUARD = '@'      ;
const char END_GUARD = ';'        ;
unsigned long lastMsgSentJason = 0;
char MY_ID[DEVICE_ID_LENGTH] = {'d', 'b', '5', 'f', '0', '4'};
#define MAX_JSON_STRING_LENGTH 100

char lastPacketId = 47;
unsigned long lastPacketRecieved = 0;
char counterPacket = 0;
const char *packetSeq = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
char packetSeqFlag = 1;
char packetLossFlag = 1;
unsigned char lossPackets[5];

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() 
{

  delay(10);
  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    //Serial.print(".");
  }

  randomSeed(micros());

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length)
{

  // Validate packet length
  if (length != PACKET_SIZE) 
  {
    //Serial.println("Invalid packet length");
    return;
  }

  // Extract payload and validate start/end guards
  char startGuard = (char)payload[0];
  char endGuard = (char)payload[PACKET_SIZE - 1];

  if (startGuard != START_GUARD || endGuard != END_GUARD) 
  {
    //Serial.println("Invalid start or end guard");
    return;
  }

  // Extract device ID
  char deviceId[DEVICE_ID_LENGTH];
  for (int i = 0; i < DEVICE_ID_LENGTH; i++) 
  {
    deviceId[i] = (char)payload[1 + i];
  }

  // Check if the received device ID matches your device ID
  if (strncmp(deviceId, MY_ID, DEVICE_ID_LENGTH) != 0) 
  {
    //Serial.println("Received packet with incorrect device ID");
    return;
  }

  // Extract packet ID and check for the seq
  char packetId = (char)payload[1 + DEVICE_ID_LENGTH];
  //checkForPacketSeq(packetId);

  // Extract MSG type
  char messageType = (char)payload[2 + DEVICE_ID_LENGTH];

  // Extract MSG data
  char msgData[4];

  //Serial.println();
  Serial.print("@");
  //Serial.print(packetId);
  Serial.print(messageType);

  for (int i = 0; i < 4; i++) 
  {
    msgData[i] = (char)payload[3 + DEVICE_ID_LENGTH + i];
    Serial.print(msgData[i]);
  }

  // Extract checksum
  char fistDigitChecksum = (char)payload[7 + DEVICE_ID_LENGTH];
  char secondDigitChecksum = (char)payload[8 + DEVICE_ID_LENGTH];

  Serial.print(fistDigitChecksum);
  Serial.print(secondDigitChecksum);

  Serial.print(";");
  

}

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    //Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect("", MQTT_USER, MQTT_PASSWORD)) 
    {
      //Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(mqtt_topic_1);
    } 
    else 
    {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() 
{
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() 
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();


  String incomingData;

  if (Serial.available()) 
  {
    incomingData = Serial.readString();
    
    // Parse incoming data and create JSON message
    if (incomingData.startsWith("@")) 
    {
      // Remove '@' character from the beginning
      incomingData.remove(0, 1);

      // Split the string by ','
      int commaIndex = incomingData.indexOf(',');
      String msgType = incomingData.substring(0, commaIndex);
      incomingData.remove(0, commaIndex + 1);
      
      commaIndex = incomingData.indexOf(',');
      String actionType = incomingData.substring(0, commaIndex);
      incomingData.remove(0, commaIndex + 1);

      DynamicJsonDocument doc(256);

      if(msgType == "E")
      {
        if(actionType == "U")
        {
          commaIndex = incomingData.indexOf(',');
          String xCoordinate = incomingData.substring(0, commaIndex);
          incomingData.remove(0, commaIndex + 1);

          String yCoordinate = incomingData.substring(0, incomingData.length() - 1); // Remove ';' character

          // Create JSON object
          
          doc["robotId"] = "db5f04";
          doc["type"] = "Error";
          doc["msg"] = "x = " + xCoordinate + " & y = " + yCoordinate;
        }
      }
      else if(msgType == "A")
      {
        if(actionType == "P")
        {
          String status = incomingData.substring(0, incomingData.length() - 1); // Remove ';' character

          
          if(status == "S")
          {
            // Create JSON object
            
            doc["robotId"] = "db5f04";
            doc["type"] = "ACK";
            doc["msg"] = "Process Success";
          }
          else if(status == "F")
          {
            // Create JSON object
            
            doc["robotId"] = "db5f04";
            doc["type"] = "ACK";
            doc["msg"] = "Process Failed";
          }
        }
      }

      // Serialize JSON to string
      String jsonString;
      serializeJson(doc, jsonString);

      // Print JSON string to Serial
      Serial.println(jsonString);

      // Publish JSON message to MQTT topic if needed
       client.publish(mqtt_topic_2, jsonString);
    }
  }
}

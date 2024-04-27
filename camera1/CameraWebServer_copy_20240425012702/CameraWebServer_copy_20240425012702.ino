#include "esp_camera.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <base64.h>

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15 
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
//#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
//#define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_DFRobot_Romeo_ESP32S3 // Has PSRAM
#include "camera_pins.h"

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
const char *mqtt_topic_1 = "18ciqt4398/robotWeb";

WiFiClient espClient;
PubSubClient client(espClient);

// ======================================
//        CAM FUNCTIONS PROTOTYPES
// ======================================
void startCameraServer();
void setupLedFlash(int pin);

// Establish MQTT connection and publish frames
void connectMQTTAndPublishFrames(uint8_t* frameBuffer, size_t frameSize) {
  WiFiClient wifiClient;
  PubSubClient mqttClient(wifiClient);

  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (mqttClient.connect("ESP32Client", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("Connected to MQTT broker!");
    } else {
      Serial.print("Failed with MQTT connection state ");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }

  mqttClient.publish(mqtt_topic_1, frameBuffer, frameSize);
}

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    
    //const char* clientId = "MazenESP8266"; // Convert String to const char*
  
    if (client.connect("", MQTT_USER, MQTT_PASSWORD)) 
    {
      //Serial.println("connected");                                         //////// "CONNECTED" TO MAKE SURE MQTT IS CONNECTED
      // Once connected, publish an announcement...
      //client.publish("18ciqt4398/temprature", "clientId=MazenESP8266");    /////// SENDING CHANNEL
      //client.publish("18ciqt4398/temprature", "I'm connected");
      // ... and resubscribe
      //client.subscribe("18ciqt4398/temprature");                           /////// RECIEVING CHANNEL
      client.subscribe(mqtt_topic_1);
      //client.subscribe(mqtt_topic_2);
      //client.subscribe(mqtt_topic_3);
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


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode (LED_GPIO_NUM, OUTPUT);//Specify that LED pin is output
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Capture a frame from the camera
  camera_fb_t* fb = NULL;
  fb = esp_camera_fb_get();
  
  if (fb) 
  {
    size_t imageSize = fb->len;
    uint8_t* imageBuffer = fb->buf;

    DynamicJsonDocument doc(1024 + imageSize);
    doc["robotId"] = "db5f04";
    String base64Encoded = base64::encode(imageBuffer, imageSize);
    doc["stream"] = base64Encoded;
    doc["camNo"] = 1;
    doc["length"] = imageSize;

    char jsonChar[1024 + imageSize];
    size_t jsonLength = serializeJson(doc, jsonChar, sizeof(jsonChar));

    client.publish(mqtt_topic_1, jsonChar, jsonLength);
    esp_camera_fb_return(fb);
  }


  // Publish the frame via MQTT
  //sendStreamMQTT(fb->buf, fb->len);
  

  // Release the frame buffer
  //esp_camera_fb_return(fb);
  //Serial.println("buff=");


  // Do nothing. Everything is done in another task by the web server
  digitalWrite(LED_GPIO_NUM, HIGH);
  delay(1000);
  //digitalWrite(LED_GPIO_NUM, LOW);
  //delay(3000);
}

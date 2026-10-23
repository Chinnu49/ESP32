#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"

// =====================================================
// DFRobot ESP32-S3 AI Camera V1.1
// XDKJ-OV3660
//
// Features:
// 1. Live video
// 2. HD photo 1280x720
// 3. Day mode
// 4. Night mode
// 5. Built-in IR illumination
// =====================================================


// =====================================================
// WIFI
// =====================================================

const char* WIFI_SSID     = "Telezer_J";
const char* WIFI_PASSWORD = "Telezer12";


// =====================================================
// CAMERA PINS
// DFRobot ESP32-S3 AI Camera V1.1
// =====================================================

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1

#define XCLK_GPIO_NUM      5

#define Y9_GPIO_NUM        4
#define Y8_GPIO_NUM        6
#define Y7_GPIO_NUM        7
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       17
#define Y4_GPIO_NUM       21
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       16

#define VSYNC_GPIO_NUM     1
#define HREF_GPIO_NUM      2
#define PCLK_GPIO_NUM     15

#define SIOD_GPIO_NUM      8
#define SIOC_GPIO_NUM      9


// =====================================================
// BOARD FUNCTIONS
// =====================================================

// DFRobot onboard IR illumination
#define IR_LED_PIN 47

// Onboard indicator LED
#define BOARD_LED_PIN 3


// =====================================================
// WEB SERVER
// =====================================================

WiFiServer server(80);


// =====================================================
// CAMERA MODE
// =====================================================

enum CameraMode
{
  DAY_MODE,
  NIGHT_MODE
};

CameraMode currentMode = DAY_MODE;


// =====================================================
// INITIALIZE CAMERA
// =====================================================

bool initCamera()
{
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  // Data pins
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  // Camera control
  config.pin_xclk  = XCLK_GPIO_NUM;
  config.pin_pclk  = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;

  // SCCB
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  // Power / reset
  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG;

  // Start live video at VGA
  config.frame_size = FRAMESIZE_VGA;

  // JPEG quality
  // Lower number = better quality / larger file
  config.jpeg_quality = 10;

  // PSRAM
  config.fb_location = CAMERA_FB_IN_PSRAM;

  config.fb_count = 2;

  config.grab_mode = CAMERA_GRAB_LATEST;


  Serial.println();
  Serial.println("--------------------------------");
  Serial.println("Initializing OV3660...");
  Serial.println("--------------------------------");


  esp_err_t err = esp_camera_init(&config);


  if (err != ESP_OK)
  {
    Serial.println();
    Serial.println("CAMERA INIT FAILED");

    Serial.print("Error code: 0x");
    Serial.println(err, HEX);

    return false;
  }


  sensor_t *sensor = esp_camera_sensor_get();


  if (sensor == NULL)
  {
    Serial.println("Sensor object not found!");
    return false;
  }


  Serial.println("Camera initialized successfully.");


  Serial.print("Camera PID: 0x");
  Serial.println(sensor->id.PID, HEX);


  if (sensor->id.PID == OV3660_PID)
  {
    Serial.println("OV3660 detected.");
  }


  // Initial image settings
  sensor->set_vflip(sensor, 1);
  sensor->set_brightness(sensor, 1);
  sensor->set_saturation(sensor, -2);


  return true;
}


// =====================================================
// DAY MODE
// =====================================================

void setDayMode()
{
  sensor_t *sensor = esp_camera_sensor_get();

  if (sensor == NULL)
    return;


  currentMode = DAY_MODE;


  // Turn OFF IR illumination
  digitalWrite(IR_LED_PIN, LOW);


  // Indicator LED
  digitalWrite(BOARD_LED_PIN, LOW);


  // Color image
  sensor->set_special_effect(sensor, 0);

  // Normal brightness
  sensor->set_brightness(sensor, 1);

  // Normal contrast
  sensor->set_contrast(sensor, 0);

  // Normal saturation
  sensor->set_saturation(sensor, 0);


  Serial.println("DAY MODE selected.");
  Serial.println("IR illumination OFF.");
}


// =====================================================
// NIGHT MODE
// =====================================================

void setNightMode()
{
  sensor_t *sensor = esp_camera_sensor_get();

  if (sensor == NULL)
    return;


  currentMode = NIGHT_MODE;


  // Turn ON IR illumination
  digitalWrite(IR_LED_PIN, HIGH);


  // Indicator LED
  digitalWrite(BOARD_LED_PIN, HIGH);


  // Increase brightness
  sensor->set_brightness(sensor, 2);

  // Increase contrast
  sensor->set_contrast(sensor, 1);

  // Reduce saturation
  sensor->set_saturation(sensor, -2);


  Serial.println("NIGHT MODE selected.");
  Serial.println("IR illumination ON.");
}


// =====================================================
// CAPTURE HD PHOTO
// =====================================================

void captureHDPhoto(WiFiClient &client)
{
  sensor_t *sensor = esp_camera_sensor_get();


  if (sensor == NULL)
  {
    client.println("HTTP/1.1 500 Internal Server Error");
    client.println("Content-Type: text/plain");
    client.println();
    client.println("Camera sensor unavailable.");
    return;
  }


  Serial.println();
  Serial.println("Capturing HD photo...");


  // Save current resolution
  framesize_t oldSize = sensor->status.framesize;


  // Change to HD
  sensor->set_framesize(sensor, FRAMESIZE_HD);


  delay(300);


  camera_fb_t *fb = esp_camera_fb_get();


  if (fb == NULL)
  {
    Serial.println("HD photo capture FAILED.");


    // Restore live video resolution
    sensor->set_framesize(sensor, FRAMESIZE_VGA);


    client.println("HTTP/1.1 500 Internal Server Error");
    client.println("Content-Type: text/plain");
    client.println();
    client.println("Photo capture failed.");

    return;
  }


  Serial.println("HD photo captured.");

  Serial.print("Width: ");
  Serial.println(fb->width);

  Serial.print("Height: ");
  Serial.println(fb->height);

  Serial.print("File size: ");
  Serial.print(fb->len);
  Serial.println(" bytes");


  // Send image
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/jpeg");
  client.println("Content-Disposition: attachment; filename=HD_photo.jpg");
  client.print("Content-Length: ");
  client.println(fb->len);
  client.println("Cache-Control: no-cache");
  client.println();


  client.write(fb->buf, fb->len);


  esp_camera_fb_return(fb);


  // Restore live video resolution
  sensor->set_framesize(sensor, FRAMESIZE_VGA);


  Serial.println("Live video restored to VGA.");
}


// =====================================================
// LIVE VIDEO STREAM
// =====================================================

void sendLiveStream(WiFiClient &client)
{
  Serial.println("Live video started.");


  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println("Cache-Control: no-cache");
  client.println("Connection: close");
  client.println();


  while (client.connected())
  {
    camera_fb_t *fb = esp_camera_fb_get();


    if (fb == NULL)
    {
      Serial.println("Frame capture failed.");
      break;
    }


    client.println("--frame");
    client.println("Content-Type: image/jpeg");

    client.print("Content-Length: ");
    client.println(fb->len);

    client.println();


    client.write(fb->buf, fb->len);

    client.println();


    esp_camera_fb_return(fb);


    delay(30);
  }


  Serial.println("Live video stopped.");
}


// =====================================================
// WEB PAGE
// =====================================================

void sendWebPage(WiFiClient &client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();


  client.println("<!DOCTYPE html>");
  client.println("<html>");

  client.println("<head>");

  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");

  client.println("<title>ESP32-S3 AI Camera</title>");


  client.println("<style>");

  client.println("body{");
  client.println("font-family:Arial;");
  client.println("text-align:center;");
  client.println("background:#111;");
  client.println("color:white;");
  client.println("margin:0;");
  client.println("padding:20px;");
  client.println("}");

  client.println("h1{font-size:28px;}");

  client.println(".camera{");
  client.println("width:100%;");
  client.println("max-width:900px;");
  client.println("margin:auto;");
  client.println("}");

  client.println("img{");
  client.println("width:100%;");
  client.println("max-width:900px;");
  client.println("border:3px solid white;");
  client.println("border-radius:10px;");
  client.println("}");

  client.println("button{");
  client.println("padding:14px 20px;");
  client.println("margin:7px;");
  client.println("font-size:16px;");
  client.println("border:0;");
  client.println("border-radius:8px;");
  client.println("cursor:pointer;");
  client.println("}");

  client.println(".day{background:#f5c542;color:black;}");
  client.println(".night{background:#555;color:white;}");
  client.println(".photo{background:#2196F3;color:white;}");
  client.println(".video{background:#4CAF50;color:white;}");

  client.println("</style>");

  client.println("</head>");


  client.println("<body>");


  client.println("<h1>ESP32-S3 AI Camera</h1>");

  client.println("<h3>XDKJ-OV3660</h3>");


  // Live video
  client.println("<div class='camera'>");

  client.println("<img src='/stream'>");

  client.println("</div>");


  // Buttons
  client.println("<div>");


  client.println("<a href='/photo'>");
  client.println("<button class='photo'>");
  client.println("📷 CAPTURE HD PHOTO");
  client.println("</button>");
  client.println("</a>");


  client.println("<a href='/day'>");
  client.println("<button class='day'>");
  client.println("☀ DAY MODE");
  client.println("</button>");
  client.println("</a>");


  client.println("<a href='/night'>");
  client.println("<button class='night'>");
  client.println("🌙 NIGHT MODE");
  client.println("</button>");
  client.println("</a>");


  client.println("</div>");


  client.println("<hr>");


  client.println("<p>");

  client.println("HD Photo: 1280 × 720");

  client.println("<br>");

  client.println("Live Video: 640 × 480");

  client.println("<br>");

  client.println("Night Mode: IR illumination enabled");

  client.println("</p>");


  client.println("</body>");

  client.println("</html>");
}


// =====================================================
// SIMPLE RESPONSE
// =====================================================

void sendMessage(WiFiClient &client, String message)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  client.println("<html>");
  client.println("<head>");
  client.println("<meta http-equiv='refresh' content='0; url=/' />");
  client.println("</head>");

  client.println("<body>");
  client.println(message);
  client.println("</body>");

  client.println("</html>");
}


// =====================================================
// HANDLE WEB REQUEST
// =====================================================

void handleWebServer()
{
  WiFiClient client = server.available();


  if (!client)
    return;


  unsigned long timeout = millis();


  while (!client.available())
  {
    if (millis() - timeout > 2000)
    {
      client.stop();
      return;
    }

    delay(1);
  }


  String request = client.readStringUntil('\r');

  client.flush();


  Serial.print("Request: ");
  Serial.println(request);


  // -------------------------------
  // LIVE VIDEO
  // -------------------------------

  if (request.indexOf("GET /stream") >= 0)
  {
    sendLiveStream(client);

    client.stop();

    return;
  }


  // -------------------------------
  // HD PHOTO
  // -------------------------------

  if (request.indexOf("GET /photo") >= 0)
  {
    captureHDPhoto(client);

    client.stop();

    return;
  }


  // -------------------------------
  // DAY MODE
  // -------------------------------

  if (request.indexOf("GET /day") >= 0)
  {
    setDayMode();

    sendMessage(client, "Day Mode Enabled");

    client.stop();

    return;
  }


  // -------------------------------
  // NIGHT MODE
  // -------------------------------

  if (request.indexOf("GET /night") >= 0)
  {
    setNightMode();

    sendMessage(client, "Night Mode Enabled");

    client.stop();

    return;
  }


  // -------------------------------
  // MAIN PAGE
  // -------------------------------

  sendWebPage(client);


  client.stop();
}


// =====================================================
// WIFI
// =====================================================

bool connectWiFi()
{
  Serial.println();
  Serial.println("Connecting WiFi...");


  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


  int count = 0;


  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);

    Serial.print(".");


    count++;


    if (count > 40)
    {
      Serial.println();
      Serial.println("WiFi connection failed.");

      return false;
    }
  }


  Serial.println();
  Serial.println();
  Serial.println("WiFi Connected");


  Serial.print("IP Address: ");

  Serial.println(WiFi.localIP());


  return true;
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);


  delay(3000);


  Serial.println();
  Serial.println();
  Serial.println("======================================");
  Serial.println(" DFRobot ESP32-S3 AI Camera V1.1");
  Serial.println(" XDKJ-OV3660");
  Serial.println("======================================");


  // =================================================
  // GPIO
  // =================================================

  pinMode(IR_LED_PIN, OUTPUT);

  pinMode(BOARD_LED_PIN, OUTPUT);


  digitalWrite(IR_LED_PIN, LOW);

  digitalWrite(BOARD_LED_PIN, LOW);


  // =================================================
  // PSRAM
  // =================================================

  Serial.println();

  Serial.println("Checking PSRAM...");


  if (psramFound())
  {
    Serial.println("PSRAM detected.");

    Serial.print("PSRAM size: ");

    Serial.print(ESP.getPsramSize() / 1024 / 1024);

    Serial.println(" MB");
  }
  else
  {
    Serial.println("WARNING: PSRAM NOT FOUND!");
  }


  // =================================================
  // CAMERA
  // =================================================

  if (!initCamera())
  {
    Serial.println();
    Serial.println("Camera initialization failed.");

    while (true)
    {
      delay(1000);
    }
  }


  // =================================================
  // DEFAULT MODE
  // =================================================

  setDayMode();


  // =================================================
  // WIFI
  // =================================================

  if (!connectWiFi())
  {
    while (true)
    {
      delay(1000);
    }
  }


  // =================================================
  // SERVER
  // =================================================

  server.begin();


  Serial.println();

  Serial.println("======================================");

  Serial.println(" CAMERA SERVER STARTED");

  Serial.println("======================================");


  Serial.print("Open browser:");

  Serial.print(" http://");

  Serial.println(WiFi.localIP());


  Serial.println();

  Serial.println("Features:");

  Serial.println("1. Live Video");

  Serial.println("2. HD Photo 1280x720");

  Serial.println("3. Day Mode");

  Serial.println("4. Night Mode");

  Serial.println("5. IR Illumination");


  Serial.println("======================================");
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  handleWebServer();
}
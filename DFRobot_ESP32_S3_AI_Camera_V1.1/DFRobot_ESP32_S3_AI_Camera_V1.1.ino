#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"

// =====================================================
// DFRobot ESP32-S3 AI Camera V1.1
// XDKJ-OV3660
// =====================================================

// ---------------- Wi-Fi ----------------
const char* WIFI_SSID     = "Telezer_J";
const char* WIFI_PASSWORD = "Telezer12";

// =====================================================
// OV3660 CAMERA PIN CONFIGURATION
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
// WEB SERVER
// =====================================================

WiFiServer server(80);


// =====================================================
// CAMERA INITIALIZATION
// =====================================================

bool initCamera()
{
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  // Camera data pins
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  // Camera control pins
  config.pin_xclk  = XCLK_GPIO_NUM;
  config.pin_pclk  = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;

  // SCCB
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  // Power/reset
  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  // Clock
  config.xclk_freq_hz = 20000000;

  // Image format
  config.pixel_format = PIXFORMAT_JPEG;

  // Start with low resolution for testing
  config.frame_size = FRAMESIZE_HD;

  // JPEG quality
  config.jpeg_quality = 10;

  // Frame buffers
  config.fb_count = 2;

  // Use PSRAM
  config.fb_location = CAMERA_FB_IN_PSRAM;

  config.grab_mode = CAMERA_GRAB_LATEST;


  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Initializing OV3660 camera...");
  Serial.println("-------------------------------------");


  esp_err_t err = esp_camera_init(&config);


  if (err != ESP_OK)
  {
    Serial.println();
    Serial.println("=====================================");
    Serial.println("       CAMERA INITIALIZATION FAILED");
    Serial.println("=====================================");

    Serial.print("Error code: 0x");
    Serial.println(err, HEX);

    Serial.println();
    Serial.println("Possible causes:");
    Serial.println("1. Camera ribbon cable problem");
    Serial.println("2. Incorrect board configuration");
    Serial.println("3. PSRAM not enabled");
    Serial.println("4. Camera pin configuration problem");
    Serial.println("5. Camera sensor not detected");

    return false;
  }


  // =================================================
  // GET CAMERA SENSOR
  // =================================================

  sensor_t *sensor = esp_camera_sensor_get();

  if (sensor == NULL)
  {
    Serial.println("ERROR: Camera sensor object is NULL.");
    return false;
  }


  Serial.println();
  Serial.println("=====================================");
  Serial.println("      CAMERA INITIALIZED SUCCESSFULLY");
  Serial.println("=====================================");

  Serial.print("Sensor PID: 0x");
  Serial.println(sensor->id.PID, HEX);


  if (sensor->id.PID == OV3660_PID)
  {
    Serial.println("OV3660 SENSOR DETECTED");

    // OV3660 image correction
    sensor->set_vflip(sensor, 1);
    sensor->set_brightness(sensor, 1);
    sensor->set_saturation(sensor, -2);
  }
  else
  {
    Serial.println("WARNING: Sensor is not identified as OV3660.");
  }


  return true;
}


// =====================================================
// CAPTURE TEST IMAGE
// =====================================================

void testCamera()
{
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Capturing test image...");
  Serial.println("-------------------------------------");


  camera_fb_t *fb = esp_camera_fb_get();


  if (fb == NULL)
  {
    Serial.println("ERROR: Image capture failed.");
    return;
  }


  Serial.println("Image capture successful.");

  Serial.print("Image width  : ");
  Serial.println(fb->width);

  Serial.print("Image height : ");
  Serial.println(fb->height);

  Serial.print("Image size   : ");
  Serial.print(fb->len);
  Serial.println(" bytes");


  esp_camera_fb_return(fb);
}


// =====================================================
// WIFI CONNECTION
// =====================================================

bool connectWiFi()
{
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Connecting WiFi...");
  Serial.println("-------------------------------------");


  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


  int attempts = 0;


  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);

    Serial.print(".");

    attempts++;


    if (attempts > 40)
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
  client.println("body{font-family:Arial;text-align:center;background:#222;color:white;}");
  client.println("img{width:95%;max-width:800px;}");
  client.println("button{padding:12px;margin:10px;font-size:18px;}");
  client.println("</style>");

  client.println("</head>");

  client.println("<body>");

  client.println("<h1>ESP32-S3 AI Camera</h1>");

  client.println("<h3>XDKJ-OV3660</h3>");

  client.println("<img src='/stream'>");

  client.println("</body>");

  client.println("</html>");
}


// =====================================================
// JPEG STREAM
// =====================================================

void sendStream(WiFiClient &client)
{
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
}


// =====================================================
// WEB SERVER
// =====================================================

void handleWebServer()
{
  WiFiClient client = server.available();


  if (!client)
  {
    return;
  }


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


  if (request.indexOf("GET /stream") >= 0)
  {
    sendStream(client);
  }
  else
  {
    sendWebPage(client);
  }


  client.stop();
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
  Serial.println("========================================");
  Serial.println(" DFRobot ESP32-S3 AI Camera V1.1");
  Serial.println(" XDKJ-OV3660");
  Serial.println(" Camera + WiFi Test");
  Serial.println("========================================");


  // =================================================
  // CHECK PSRAM
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
    Serial.println("Camera may not initialize correctly.");
  }


  // =================================================
  // INITIALIZE CAMERA FIRST
  // =================================================

  if (!initCamera())
  {
    Serial.println();
    Serial.println("Camera initialization failed.");
    Serial.println();
    Serial.println("STOPPING PROGRAM.");
    Serial.println("WiFi will NOT be started.");

    while (true)
    {
      delay(1000);
    }
  }


  // =================================================
  // CAMERA TEST
  // =================================================

  testCamera();


  // =================================================
  // CONNECT WIFI
  // =================================================

  if (!connectWiFi())
  {
    Serial.println("WiFi failed.");

    while (true)
    {
      delay(1000);
    }
  }


  // =================================================
  // START SERVER
  // =================================================

  server.begin();


  Serial.println();
  Serial.println("========================================");
  Serial.println("       CAMERA SERVER STARTED");
  Serial.println("========================================");

  Serial.print("Open browser: http://");
  Serial.println(WiFi.localIP());

  Serial.println();
  Serial.println("Live stream:");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");

  Serial.println();
  Serial.println("========================================");
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  handleWebServer();
}
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// ================= WIFI =================
const char* ssid = "Telezer_J";
const char* password = "Telezer12";

// ================= CAMERA PINS (ESP32-S3 N16R8 COMMON OV2640) =================
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    15
#define SIOD_GPIO_NUM    4
#define SIOC_GPIO_NUM    5

#define Y9_GPIO_NUM      16
#define Y8_GPIO_NUM      17
#define Y7_GPIO_NUM      18
#define Y6_GPIO_NUM      12
#define Y5_GPIO_NUM      10
#define Y4_GPIO_NUM      8
#define Y3_GPIO_NUM      9
#define Y2_GPIO_NUM      11
#define VSYNC_GPIO_NUM   6
#define HREF_GPIO_NUM    7
#define PCLK_GPIO_NUM    13

httpd_handle_t stream_httpd = NULL;

// ================= STREAM HANDLER =================
static esp_err_t stream_handler(httpd_req_t *req)
{
  camera_fb_t * fb = NULL;

  httpd_resp_set_type(req,
    "multipart/x-mixed-replace;boundary=frame");

  while (true)
  {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return ESP_FAIL;
    }

    httpd_resp_send_chunk(req, "--frame\r\n", 8);
    httpd_resp_send_chunk(req,
      "Content-Type: image/jpeg\r\n\r\n", 28);
    httpd_resp_send_chunk(req,
      (const char *)fb->buf, fb->len);
    httpd_resp_send_chunk(req, "\r\n", 2);

    esp_camera_fb_return(fb);
  }

  return ESP_OK;
}

// ================= START SERVER =================
void startCameraServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  httpd_start(&stream_httpd, &config);

  httpd_uri_t uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  httpd_register_uri_handler(stream_httpd, &uri);

  Serial.println("Camera Web Server Started");
}

// ================= SETUP =================
void setup()
{
  Serial.begin(115200);
  Serial.println();

  // WIFI CONNECT
  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // CAMERA CONFIG
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

  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  // INIT CAMERA
  if (esp_camera_init(&config) != ESP_OK)
  {
    Serial.println("Camera Init Failed");
    return;
  }

  Serial.println("Camera Ready!");

  // START SERVER
  startCameraServer();
}

void loop()
{
  delay(10000);
}

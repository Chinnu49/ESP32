#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

#define LED_PIN 2

String ssidInput = "";
String passInput = "";
bool connected = false;

// ================= AP PAGE =================
void apPage()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>WiFi Setup</title>
<style>
body{font-family:Arial;text-align:center;background:#f2f2f2;}
.box{background:white;width:300px;margin:auto;margin-top:80px;padding:20px;border-radius:10px;}
input{width:90%;padding:10px;margin:10px;}
button{padding:10px;width:95%;background:green;color:white;border:none;}
</style>
</head>
<body>

<div class="box">
<h2>ESP32 WiFi Setup</h2>

<form action="/save" method="POST">
<input name="ssid" placeholder="WiFi Name">
<input name="pass" placeholder="WiFi Password" type="password">
<button type="submit">Connect</button>
</form>

</div>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

// ================= SAVE WIFI =================
void saveWiFi()
{
  String ssidInput = server.arg("ssid");
  String passInput = server.arg("pass");

  WiFi.softAPdisconnect(true);
  WiFi.begin(ssidInput.c_str(), passInput.c_str());

  int count = 0;

  while (WiFi.status() != WL_CONNECTED && count < 20)
  {
    delay(500);
    count++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    String ip = WiFi.localIP().toString();

    String page = "<!DOCTYPE html><html><head>";
    page += "<style>body{text-align:center;font-family:Arial;background:#f2f2f2;}";
    page += ".box{background:white;width:300px;margin:auto;margin-top:100px;padding:20px;border-radius:10px;}";
    page += "a button{padding:15px;width:200px;background:green;color:white;border:none;font-size:16px;}";
    page += "</style></head><body>";

    page += "<div class='box'>";
    page += "<h2>WiFi Connected Successfully</h2>";
    page += "<p><b>IP Address:</b> " + ip + "</p>";

    page += "<a href='/control'>";
    page += "<button>Go to Control Page</button>";
    page += "</a>";

    page += "</div></body></html>";

    server.send(200, "text/html", page);
  }
  else
  {
    server.send(200, "text/html",
      "<h2>Connection Failed</h2><p>Try again</p><a href='/'>Back</a>");
  }
}
// ================= CONTROL PAGE =================
void controlPage()
{
  String ip = WiFi.localIP().toString();

  String html = "<!DOCTYPE html><html><head>";
  html += "<style>body{text-align:center;font-family:Arial;}";
  html += "button{padding:20px;width:150px;margin:10px;font-size:18px;}";
  html += "</style></head><body>";

  html += "<h2>LED CONTROL</h2>";
  html += "<p>IP: " + ip + "</p>";

  html += "<a href='/on'><button style='background:green;color:white;'>ON</button></a>";
  html += "<a href='/off'><button style='background:red;color:white;'>OFF</button></a>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

// ================= LED CONTROL =================
void ledOn()
{
  digitalWrite(LED_PIN, HIGH);
  server.sendHeader("Location", "/control");
  server.send(303);
}

void ledOff()
{
  digitalWrite(LED_PIN, LOW);
  server.sendHeader("Location", "/control");
  server.send(303);
}

// ================= SETUP =================
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Start AP mode first
  WiFi.softAP("ESP32_Setup", "12345678");

  Serial.println("AP Started");
  Serial.println(WiFi.softAPIP());

  server.on("/", apPage);
  server.on("/save", HTTP_POST, saveWiFi);
  server.on("/control", controlPage);
  server.on("/on", ledOn);
  server.on("/off", ledOff);

  server.begin();
}

// ================= LOOP =================
void loop()
{
  server.handleClient();
}
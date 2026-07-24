#include <WiFi.h>

const char* ssid     = "Telezer_J";
const char* password = "Telezer12";

WiFiServer server(80);
const int ledPin = 2;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected.");
  
  // Start the server and print IP address
  server.begin();
  Serial.print("IP address to visit: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Send standard HTTP response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Build the HTML Web Page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>body{text-align:center; font-family:sans-serif;} .btn{padding:20px 40px; font-size:24px; color:white; border:none; text-decoration:none; margin:10px; cursor:pointer;} .on{background-color:green;} .off{background-color:red;}</style></head>");
            client.println("<body><h1>ESP32 Control Page</h1>");
            client.println("<p><a href=\"/H\"><button class=\"btn on\">TURN ON</button></a></p>");
            client.println("<p><a href=\"/L\"><button class=\"btn off\">TURN OFF</button></a></p>");
            client.println("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        // Process requests from the buttons
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(ledPin, HIGH);
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(ledPin, LOW);
        }
      }
    }
    client.stop();
  }
}

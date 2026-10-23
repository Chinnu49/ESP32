#include "WiFi.h"

#define ONBOARD_LED 8

void setup() {
  Serial.begin(115200);
  pinMode(ONBOARD_LED, OUTPUT);
  
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup completed! Starting network scan...");
}

void loop() {
  Serial.println("Scanning for networks...");
  
  // Blink the LED once to show a scan is happening
  digitalWrite(ONBOARD_LED, LOW); 
  delay(100);
  digitalWrite(ONBOARD_LED, HIGH);

  int n = WiFi.scanNetworks();
  Serial.println("Scan done!");
  
  if (n == 0) {
    Serial.println("No networks found.");
  } else {
    Serial.printf("%d networks found:\n", n);
    for (int i = 0; i < n; ++i) {
      // Print network name and signal strength (RSSI)
      Serial.printf("%d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
      delay(10);
    }
  }
  Serial.println("");

  // Wait 5 seconds before scanning again
  delay(5000);
}

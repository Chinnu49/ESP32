#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 3; 
BLEScan* pBLEScan;
const int buzzerPin = 4;

// ⚠️ Ensure this matches your watch's address exactly
const String TARGET_MAC = "change_this_to_your_watch_mac"; 

// -64 is greater than -82, meaning it is safely nearby!
const int RSSI_THRESHOLD = -62; 
volatile bool deviceFoundGlobal = false;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      String foundAddress = advertisedDevice.getAddress().toString().c_str();
      
      if (foundAddress == TARGET_MAC) {
        int rssi = advertisedDevice.getRSSI();
        Serial.print("-> Target spotted! Current RSSI: ");
        Serial.println(rssi);
        
        // Logical Fix: If signal is stronger than threshold, it is safe
        if (rssi >= RSSI_THRESHOLD) {
          deviceFoundGlobal = true;
        }
      }
    }
};

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin); 

  Serial.println("--- System Initializing ---");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); 
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); 
  pBLEScan->setInterval(160); // Optimized timing intervals for S3 BLE stack
  pBLEScan->setWindow(120);  
}

void loop() {
  Serial.println("\n[Scan Start] Searching for your watch...");
  deviceFoundGlobal = false;
  
  // Run the hardware scan engine cleanly
  pBLEScan->start(scanTime, false);
  
  // Fixed: Do not call pBLEScan->stop() manually to prevent the BT_BTM scan driver error
  
  if (deviceFoundGlobal == false) {
    Serial.println("⚠️ RESULT: Watch is OUT OF RANGE or missing! Sounding alarm!");
    
    // Play sound pattern on the buzzer
    for(int i = 0; i < 2; i++) {
      tone(buzzerPin, 1000); 
      delay(150);
      tone(buzzerPin, 1500); 
      delay(150);
    }
    noTone(buzzerPin); 
    
  } else {
    Serial.println("✅ RESULT: Watch is safely NEARBY. Silence maintained.");
    noTone(buzzerPin); 
  }
  
  pBLEScan->clearResults();   
  Serial.println("[Scan End] Memory flushed. Waiting for next cycle...");
  delay(2000); 
}

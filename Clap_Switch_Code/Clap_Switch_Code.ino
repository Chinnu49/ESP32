// Pin Definitions
#define SOUND_PIN 34  // Analog Output (AO) of Big Sound Sensor
#define LED_PIN   2   // GPIO 2 (Built-in or External LED)

// Adjust this threshold to make the sensor more or less sensitive to claps
// Higher number = needs a louder clap. Lower number = triggers more easily.
const int CLAP_THRESHOLD = 800; 

// Timing variables to prevent a single clap from triggering the LED multiple times
const int sampleWindow = 40; 
unsigned long lastClapTime = 0;
const int relayDelay = 400; // Wait 400ms before accepting another clap

bool ledState = false; // Tracks if the LED is currently ON or OFF

void setup() {
  Serial.begin(115200);
  pinMode(SOUND_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState); // Start with LED off
  Serial.println("Clap Switch Ready! Clap to toggle the LED.");
}

void loop() {
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 4095;
  unsigned int sample;

  // Collect audio samples for 40 milliseconds
  while (millis() - startMillis < sampleWindow) {
    sample = analogRead(SOUND_PIN);
    if (sample < 4095) {
      if (sample > signalMax) signalMax = sample;
      if (sample < signalMin) signalMin = sample;
    }
  }

  // Calculate total volume amplitude of this short window
  unsigned int peakToPeak = signalMax - signalMin;

  // Check if sound spikes past the threshold and enough time has passed since the last clap
  if (peakToPeak > CLAP_THRESHOLD && (millis() - lastClapTime > relayDelay)) {
    ledState = !ledState;            // Reverse the LED state
    digitalWrite(LED_PIN, ledState); // Apply the new state to the physical pin
    
    Serial.print("Clap Detected! Amplitude: ");
    Serial.print(peakToPeak);
    Serial.println(ledState ? " -> LED turned ON" : " -> LED turned OFF");
    
    lastClapTime = millis(); // Record the timestamp of this clap
  }
}

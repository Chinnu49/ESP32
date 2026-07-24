#include <BleKeyboard.h>

BleKeyboard bleKeyboard("ESP32 Media Controller", "OpenAI", 100);

// Button Pins
const int PLAY_BUTTON = 12;
const int NEXT_BUTTON = 13;
const int PREV_BUTTON = 14;
const int VOLUP_BUTTON = 27;
const int VOLDOWN_BUTTON = 26;

bool lastPlay = HIGH;
bool lastNext = HIGH;
bool lastPrev = HIGH;
bool lastVolUp = HIGH;
bool lastVolDown = HIGH;

void setup() {
  Serial.begin(115200);

  pinMode(PLAY_BUTTON, INPUT_PULLUP);
  pinMode(NEXT_BUTTON, INPUT_PULLUP);
  pinMode(PREV_BUTTON, INPUT_PULLUP);
  pinMode(VOLUP_BUTTON, INPUT_PULLUP);
  pinMode(VOLDOWN_BUTTON, INPUT_PULLUP);

  Serial.println("Starting Bluetooth Media Controller...");

  bleKeyboard.begin();
}

void loop() {

  if (!bleKeyboard.isConnected()) {
    delay(100);
    return;
  }

  bool playState = digitalRead(PLAY_BUTTON);
  bool nextState = digitalRead(NEXT_BUTTON);
  bool prevState = digitalRead(PREV_BUTTON);
  bool volUpState = digitalRead(VOLUP_BUTTON);
  bool volDownState = digitalRead(VOLDOWN_BUTTON);

  // Play / Pause
  if (playState == LOW && lastPlay == HIGH) {
    Serial.println("Play/Pause");
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
    delay(200);
  }

  // Next Track
  if (nextState == LOW && lastNext == HIGH) {
    Serial.println("Next Track");
    bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
    delay(200);
  }

  // Previous Track
  if (prevState == LOW && lastPrev == HIGH) {
    Serial.println("Previous Track");
    bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
    delay(200);
  }

  // Volume Up
  if (volUpState == LOW && lastVolUp == HIGH) {
    Serial.println("Volume Up");
    bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    delay(200);
  }

  // Volume Down
  if (volDownState == LOW && lastVolDown == HIGH) {
    Serial.println("Volume Down");
    bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    delay(200);
  }

  lastPlay = playState;
  lastNext = nextState;
  lastPrev = prevState;
  lastVolUp = volUpState;
  lastVolDown = volDownState;

  delay(20);
}
/* 
 * ManyKey firmware
 * See README.md
 * See LICENSE (MIT)
*/

#include <Keyboard.h>
#include <EEPROM.h>

#define DEBOUNCE_DELAY 10
#define MAX_CHARS_PER_BUTTON 25
#define SERIAL_BUFFER_LENGTH 20
#define SERIAL_BAUD_RATE 9600
#define EEPROM_INTEGRITY_BYTE 123
#define EEPROM_DATA_START 0x10

/* EEPROM that decides if keys on the macropad should be pressed in sequence */
/* TODO: Update this to be on a per key basis */
#define EEPROM_IN_KEY_SEQUENCE_MODE_ADDRESS 0x03

/* EEPROM that decides if keys built in LEDs (if any) should be on */
/* TODO: Update this to be on a per key basis */
#define EEPROM_LED_ON_ADDRESS 0x04

#define SERIAL_START_BYTE 0xEE
#define SERIAL_END_BYTE 0xFF
#define SERIAL_READ_COMMAND 0x00
#define SERIAL_WRITE_COMMAND 0x01
#define SERIAL_QUERY_SETTINGS_COMMAND 0x02


/* --------- Button declarations and functions */
/* Edit list of pins and count here */
#define BUTTON_COUNT 5
byte buttonPins[] = {9, 8, 7, 5, 6};

boolean inKeySequenceMode = false;

// Serial Global Variables
byte serialReadBuffer[SERIAL_BUFFER_LENGTH];
byte serialWriteBuffer[SERIAL_BUFFER_LENGTH];
bool dataAvailable = false;

typedef struct {
  bool state;
  bool lastReading;
  bool latched;
  unsigned long lastTime;
  byte chars[MAX_CHARS_PER_BUTTON];
  byte pin;
  byte index;
} button;

button buttons[BUTTON_COUNT];
bool buttonReadings[BUTTON_COUNT];

void initButtons(){
  for (byte i = 0; i < BUTTON_COUNT; i++) {
    buttons[i].state = false;
    buttons[i].lastReading = false;
    buttons[i].latched = false;
    buttons[i].lastTime = 0;
    wipeArray(buttons[i].chars, MAX_CHARS_PER_BUTTON);
    buttons[i].chars[0] = 97 + i;
    buttons[i].pin = buttonPins[i];
    pinMode(buttons[i].pin, INPUT_PULLUP);
    buttons[i].index = i;
  }
}

void pressChars(button btn){
  for (byte i = 0; i < MAX_CHARS_PER_BUTTON; i++) {
    Keyboard.press(btn.chars[i]);
    if(inKeySequenceMode){
      delay(1);
      Keyboard.release(btn.chars[i]);
      delay(1);
    }
  }
}

void releaseChars(button btn){
  for (byte i = 0; i < MAX_CHARS_PER_BUTTON; i++) {
    if(!inKeySequenceMode){
      Keyboard.release(btn.chars[i]);
    }
  }
}

void updateButtons() {
  for (byte i = 0; i < BUTTON_COUNT; i++){
    // update button readings
    buttonReadings[i] = !digitalRead(buttons[i].pin);
    if (buttonReadings[i] != buttons[i].lastReading) {
      buttons[i].lastTime = millis();
    }

    // press/release buttons
    if ((millis() - buttons[i].lastTime) > DEBOUNCE_DELAY){
      buttons[i].state = buttonReadings[i];
      if (buttons[i].state && !buttons[i].latched) {
        pressChars(buttons[i]);
        buttons[i].latched = true;
      } else if (!buttons[i].state && buttons[i].latched) {
        releaseChars(buttons[i]);
        buttons[i].latched = false;
      }
    }

    // update last reading
    buttons[i].lastReading = buttonReadings[i];
  }
}

/* ---------------------------- Misc functions */
void wipeArray(byte *arr, int len){
  for (int i = 0; i < len; i++){
    arr[i] = 0;   
  }
}


/* ---------------------------- Setup and loop */
void setup() {
  Keyboard.begin();
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.setTimeout(10);
  initButtons();
  loadConfigFromEEPROM();
}

void loop() {
  updateButtons();
  
  if (dataAvailable) {
    processSerialBuffer();
  } else {
    readSerial();
  }
}

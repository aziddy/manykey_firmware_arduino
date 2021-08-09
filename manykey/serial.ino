/* --------------- Serial read/write functions */


void readSerial(){
  if (Serial.available() && !dataAvailable){
    Serial.readBytes(serialWriteBuffer, SERIAL_BUFFER_LENGTH);
    dataAvailable = true;
  }
}

// not super happy with how this reads, probably an area for refactor
void processSerialBuffer(){
  if (serialWriteBuffer[0] == SERIAL_START_BYTE) { // start byte
    if (serialWriteBuffer[1] == SERIAL_READ_COMMAND) { // command byte
      processSerialRead();
    } else if (serialWriteBuffer[1] == SERIAL_WRITE_COMMAND) { // command byte
      processSerialWrite();
    } else if (serialWriteBuffer[1] == SERIAL_QUERY_SETTINGS_COMMAND){
      for (byte i = 2; i < SERIAL_BUFFER_LENGTH; i++){
        if (serialWriteBuffer[i] == SERIAL_END_BYTE){ // end byte
          writeSerialQuery();
          break;
        }
      }
    }
  }
  discardSerialBuffer();
}

/* Read Saved Key Values */
void processSerialRead(){
  if (parsedIndexValid(serialWriteBuffer[2])){ // button index byte
    for (byte i = 3; i < SERIAL_BUFFER_LENGTH; i++){
      if (serialWriteBuffer[i] == SERIAL_END_BYTE) { // end byte
        writeSerialSwitchStatus(buttons[serialWriteBuffer[2]], SERIAL_READ_COMMAND);
        break;
      }
    }
  }
}

void processSerialWrite(){
  if (parsedIndexValid(serialWriteBuffer[2])){ // button index byte
    byte newChars[MAX_CHARS_PER_BUTTON];
    wipeArray(newChars, MAX_CHARS_PER_BUTTON);
    for (byte i = 3; i < SERIAL_BUFFER_LENGTH; i++){
      if (serialWriteBuffer[i] == SERIAL_END_BYTE){ // end byte
        for (byte j = 0; j < MAX_CHARS_PER_BUTTON; j++){ // write new chars
          buttons[serialWriteBuffer[2]].chars[j] = newChars[j];
        }
        saveConfigToEEPROM();
        writeSerialSwitchStatus(buttons[serialWriteBuffer[2]], SERIAL_WRITE_COMMAND);
        break;
      }
      newChars[i - 3] = serialWriteBuffer[i];
    }
  }
}

void writeSerialSwitchStatus(button btn, byte command){
  wipeArray(serialWriteBuffer, SERIAL_BUFFER_LENGTH);
  serialWriteBuffer[0] = 0xEE;
  serialWriteBuffer[1] = command;
  serialWriteBuffer[2] = btn.index;
  byte i = 0;
  while (i < MAX_CHARS_PER_BUTTON){
    if (btn.chars[i] == 0x00){ break; }
    serialWriteBuffer[i+3] = btn.chars[i];
    i++;
  }
  serialWriteBuffer[i+3] = 0xFF;
  Serial.write(serialWriteBuffer, i+4);
}

void writeSerialQuery(){
  wipeArray(serialWriteBuffer, SERIAL_BUFFER_LENGTH);
  serialWriteBuffer[0] = 0xEE;
  serialWriteBuffer[1] = SERIAL_QUERY_SETTINGS_COMMAND;
  serialWriteBuffer[2] = BUTTON_COUNT;
  serialWriteBuffer[3] = MAX_CHARS_PER_BUTTON;
  serialWriteBuffer[4] = 0xFF;
  Serial.write(serialWriteBuffer, 5);
}

void discardSerialBuffer(){
  wipeArray(serialWriteBuffer, SERIAL_BUFFER_LENGTH);
  dataAvailable = false;
}

bool parsedIndexValid(byte index){
  if (index < BUTTON_COUNT) {
    return true;
  } else {
    return false;
  }
}

void wipeEEPROM(){
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
}

void saveConfigToEEPROM(){
  EEPROM.update(0x00, EEPROM_INTEGRITY_BYTE);
  EEPROM.update(0x01, MAX_CHARS_PER_BUTTON);
  EEPROM.update(0x02, BUTTON_COUNT);
  for (byte i = 0; i < BUTTON_COUNT; i++){
    for (byte j = 0; j < MAX_CHARS_PER_BUTTON; j++){
      int address = EEPROM_DATA_START + (i * MAX_CHARS_PER_BUTTON) + j;
      EEPROM.update(address, buttons[i].chars[j]);
    }
  }
}

void loadConfigFromEEPROM(){
  if ((EEPROM.read(0x00) == EEPROM_INTEGRITY_BYTE) &&
      (EEPROM.read(0x01) == MAX_CHARS_PER_BUTTON) &&
      (EEPROM.read(0x02) == BUTTON_COUNT)) {
    for (byte i = 0; i < BUTTON_COUNT; i++){
      for (byte j = 0; j < MAX_CHARS_PER_BUTTON; j++){
        int address = EEPROM_DATA_START + (i * MAX_CHARS_PER_BUTTON) + j;
        buttons[i].chars[j] = EEPROM.read(address);
      }
    }
  } else {
    wipeEEPROM();
    saveConfigToEEPROM();
  }
}


void loadKeySequenceModeFromEEPROM(){
  EEPROM.get(EEPROM_IN_KEY_SEQUENCE_MODE_ADDRESS, inKeySequenceMode);
}

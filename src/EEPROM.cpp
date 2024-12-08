//
// Created by maxim on 08.12.2024.
//

#include "EEPROM.h"

Preferences EEPROM::preferences = Preferences();

void EEPROM::init(String name) {
    preferences.begin(name.c_str(), false);
}

void EEPROM::storeUInt(String key, unsigned int value) {
    preferences.putUInt(key.c_str(), value);
}

unsigned int EEPROM::readUInt(String key) {
    return preferences.getUInt(key.c_str(), 0);
}
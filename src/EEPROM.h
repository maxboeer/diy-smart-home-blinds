//
// Created by maxim on 08.12.2024.
//

#ifndef DIY_SMART_HOME_BLINDS_EEPROM_H
#define DIY_SMART_HOME_BLINDS_EEPROM_H
#include <Arduino.h>
#include <Preferences.h>

class EEPROM {
public:
    static void init(String name);
    static void storeUInt(String key, unsigned int value);
    static unsigned int readUInt(String key);
private:
    static Preferences preferences;
};


#endif //DIY_SMART_HOME_BLINDS_EEPROM_H

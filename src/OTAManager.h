//
// Created by maxim on 07.01.2026.
//

#ifndef DIY_SMART_HOME_BLINDS_OTAMANAGER_H
#define DIY_SMART_HOME_BLINDS_OTAMANAGER_H

#include <Arduino.h>

class OTAManager {
public:
    static void setup(const String& hostname, const uint16_t& port, const String& password);
    static void handle();
};



#endif //DIY_SMART_HOME_BLINDS_OTAMANAGER_H
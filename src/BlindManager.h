//
// Created by maxim on 05.12.2024.
//

#ifndef DIY_SMART_HOME_BLINDS_BLINDMANAGER_H
#define DIY_SMART_HOME_BLINDS_BLINDMANAGER_H

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#include "Blind.h"
#include <vector>

class BlindManager {
public:
    BlindManager(int powerOnPin, int powerControlPin);
    void addBlind(int dir_pin, int step_pin, int position, int top_steps, int bottom_steps, const String& BlindID);
    void handle();
    std::vector<Blind*> blinds;
private:
    void powerOn();
    void powerOff();

    int powerOnPin;
    int powerControlPin;
};


#endif //DIY_SMART_HOME_BLINDS_BLINDMANAGER_H

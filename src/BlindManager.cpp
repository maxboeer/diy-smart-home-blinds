//
// Created by maxim on 05.12.2024.
//

#include "BlindManager.h"

BlindManager::BlindManager(int powerOnPin, int powerControlPin) : powerOnPin(powerOnPin), powerControlPin(powerControlPin) {
    pinMode(powerOnPin, OUTPUT);
    digitalWrite(powerOnPin, HIGH);
    pinMode(powerControlPin, INPUT_PULLUP);
}

void BlindManager::addBlind(int dir_pin, int step_pin, int position, int top_steps, int bottom_steps, const String &BlindID) {
    blinds.push_back(new Blind(dir_pin, step_pin, position, top_steps, bottom_steps, BlindID));
}

void BlindManager::handle() {
    if (blinds_running) {
        bool someDelta = false;
        for (auto blind : blinds) {
            if (blind->delta_steps != 0){
                powerOn();
                someDelta = true;
            }
            blind->doTick();
        }
        if (!someDelta) powerOff();
    }
}

void BlindManager::powerOn() {
    digitalWrite(powerOnPin, false);
    // Wait until the power control pin is high
    while (!digitalRead(powerControlPin)){
        digitalWrite(LED_BUILTIN, false);}
}

void BlindManager::powerOff(){
    digitalWrite(powerOnPin, true);
}
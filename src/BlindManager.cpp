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
    bool someDelta = false;
    for (auto blind : blinds) {
        if (!blind->target_positions.empty()) {
            if (blind->position == blind->target_positions.back()){
                blind->iterations = 0;
                blind->last_target_position = blind->target_positions.back();
                blind->was_running = false;
                while (!blind->target_positions.empty())
                    blind->target_positions.pop();
                continue;
            }
            powerOn();
            someDelta = true;
            blind->doTick();
        }
    }
    if (!someDelta) powerOff();
}

void BlindManager::powerOn() {
    digitalWrite(powerOnPin, LOW);
    // Wait until the power control pin is high
    while (!digitalRead(powerControlPin)){
        digitalWrite(LED_BUILTIN, LOW);}
    digitalWrite(LED_BUILTIN, HIGH);
}

void BlindManager::powerOff() {
    digitalWrite(powerOnPin, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
}
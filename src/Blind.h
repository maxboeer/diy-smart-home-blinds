//
// Created by maxim on 05.12.2024.
//

#ifndef DIY_SMART_HOME_BLINDS_BLIND_H
#define DIY_SMART_HOME_BLINDS_BLIND_H
#include <Arduino.h>
#include "SinricHandler.h"
#include "Lookups.cpp"

class Blind {
public:
    Blind(int dir_pin, int step_pin, int position, int top_steps, int bottom_steps, const String& BlindID, const Lookups::accelLookup<(unsigned int)Lookups::L_accel>* accel_lookup = &Lookups::accel_lookup, const Lookups::decelLookup<(unsigned int)Lookups::L_decel>* decel_lookup = &Lookups::decel_lookup);
    void doTick();
    SINRICPRO_NAMESPACE::SinricProBlinds* sinricBlind;
    int position;
    int delta_steps = 0;
    int top_steps;
    int bottom_steps;
private:
    void step(bool dir);
    int dir_pin;
    int step_pin;
    unsigned long last_step_time = 0;
    unsigned int iterations = 0;
    const Lookups::accelLookup<(unsigned int)Lookups::L_accel>* accel_lookup;
    const Lookups::decelLookup<(unsigned int)Lookups::L_decel>* decel_lookup;
};


#endif //DIY_SMART_HOME_BLINDS_BLIND_H

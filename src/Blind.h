//
// Created by maxim on 05.12.2024.
//

#ifndef DIY_SMART_HOME_BLINDS_BLIND_H
#define DIY_SMART_HOME_BLINDS_BLIND_H
#include <Arduino.h>
#include <queue>
#include "SinricHandler.h"
#include "Lookups.cpp"

class Blind {
public:
    Blind(int dir_pin, int step_pin, int position, int top_steps, int bottom_steps, const String& BlindID, const Lookups::accelLookup<(unsigned int)Lookups::L_accel>* accel_lookup = &Lookups::accel_lookup, const Lookups::decelLookup<(unsigned int)Lookups::L_decel>* decel_lookup = &Lookups::decel_lookup);
    void doTick();
    SINRICPRO_NAMESPACE::SinricProBlinds* sinricBlind;
    int position;
    std::queue<int> target_positions;
    int top_steps;
    int bottom_steps;
    int id;

    int last_target_position;
    bool was_running = false;
    unsigned int iterations;
private:
    void step(bool dir);
    int dir_pin;
    int step_pin;
    const Lookups::accelLookup<(unsigned int)Lookups::L_accel>* accel_lookup;
    const Lookups::decelLookup<(unsigned int)Lookups::L_decel>* decel_lookup;
    unsigned long last_step_time;
    double steptime;
    static int blindCount;
};


#endif //DIY_SMART_HOME_BLINDS_BLIND_H

//
// Created by maxim on 05.12.2024.
//

#ifndef DIY_SMART_HOME_BLINDS_BLIND_H
#define DIY_SMART_HOME_BLINDS_BLIND_H
#include <Arduino.h>
#include "SinricHandler.h"

class Blind {
public:
    Blind(int dir_pin, int step_pin, int position, int top_steps, int bottom_steps, const String& BlindID);
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
    double steptime_high = 3000;
    double steptime_low = 100;
    double interpolation_length = 500;
//    double interpolation_factor = log10(steptime_low/steptime_high) / (interpolation_length * log10(double(1)/double(2)));
    unsigned int iterations = 0;
//    bool last_direction = true;
};


#endif //DIY_SMART_HOME_BLINDS_BLIND_H

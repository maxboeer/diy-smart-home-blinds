//
// Created by maxim on 05.12.2024.
//

#ifndef DIY_SMART_HOME_BLINDS_BLIND_H
#define DIY_SMART_HOME_BLINDS_BLIND_H
#include <Arduino.h>
#include <queue>
#include "SinricHandler.h"
#include "MotionPlanner.h"

class Blind {
public:
    Blind(int dir_pin, int step_pin, int position, int top_steps, int bottom_steps, const String& BlindID);
    void doTick();
    bool wantsMotion() const;
    SINRICPRO_NAMESPACE::SinricProBlinds* sinricBlind;
    int position;
    std::queue<int> target_positions;
    int top_steps;
    int bottom_steps;
    int id;

    int last_target_position;
private:
    void step(bool dir);
    bool canStep(bool dir) const;
    int dir_pin;
    int step_pin;
    MotionPlanner planner;
    double stepAccumulator;
    unsigned long lastUpdateMicros;
    static int blindCount;
};


#endif //DIY_SMART_HOME_BLINDS_BLIND_H

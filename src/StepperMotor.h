//
// Created by maxim on 05.12.2024.
//

#ifndef DIY_SMART_HOME_BLINDS_STEPPERMOTOR_H
#define DIY_SMART_HOME_BLINDS_STEPPERMOTOR_H
#include <Arduino.h>

class StepperMotor {
public:
    StepperMotor(int dir_pin, int step_pin, double steptime_high, double steptime_low, double interpolation_length, double interpolation_factor);
    void step(bool dir);
private:
    int dir_pin;
    int step_pin;
    double steptime_high;
    double steptime_low;
    double interpolation_length;
    double interpolation_factor;
};


#endif //DIY_SMART_HOME_BLINDS_STEPPERMOTOR_H

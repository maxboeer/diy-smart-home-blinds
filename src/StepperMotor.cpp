//
// Created by maxim on 05.12.2024.
//

#include "StepperMotor.h"

StepperMotor::StepperMotor(int dir_pin, int step_pin, double steptime_high, double steptime_low, double interpolation_length, double interpolation_factor){
    this->dir_pin = dir_pin;
    this->step_pin = step_pin;
    this->steptime_high = steptime_high;
    this->steptime_low = steptime_low;
    this->interpolation_length = interpolation_length;
    this->interpolation_factor = interpolation_factor;
}

void StepperMotor::step(bool dir){
    digitalWrite(this->dir_pin, dir);
    digitalWrite(this->step_pin, HIGH);
    digitalWrite(this->step_pin, LOW);
}
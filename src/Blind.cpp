//
// Created by maxim on 05.12.2024.
//

#include "Blind.h"

Blind::Blind(int dir_pin, int step_pin, int position, int top_steps, int bottom_steps, const String& BlindID, const Lookups::accelLookup<(unsigned int)Lookups::L_accel>* accel_lookup, const Lookups::decelLookup<(unsigned int)Lookups::L_decel>* decel_lookup) {
    this->dir_pin = dir_pin;
    this->step_pin = step_pin;
    this->position = position;
    this->top_steps = top_steps;
    this->bottom_steps = bottom_steps;
    this->sinricBlind = SinricHandler::getBlind(BlindID);
    this->accel_lookup = accel_lookup;
    this->decel_lookup = decel_lookup;

    pinMode(dir_pin, OUTPUT);
    pinMode(step_pin, OUTPUT);
    digitalWrite(step_pin, 0);
}

void Blind::step(bool dir){
    digitalWrite(this->dir_pin, dir);
    digitalWrite(this->step_pin, HIGH);
    digitalWrite(this->step_pin, LOW);
}

void Blind::doTick() {
    unsigned long current_micros = micros(); // Get the current time in microseconds
    if (delta_steps == 0) { // If there are no steps to perform, return
        iterations = 0; // Reset the iteration count
        return;
    }

    // Calculate normalized time (0 to 1)
    double x = double(iterations);
    double speed = 0.0;

    // Acceleration phase
    if (x <= accel_lookup->length) {
        speed = accel_lookup->arr[int(x)];
    }
    // Deceleration phase
    else if (x >= (abs(delta_steps) - decel_lookup->length)) {
        double x_decel = abs(delta_steps) - x;
        speed = decel_lookup->arr[int(x_decel)];
    }
    // Constant speed phase
    else {
        speed = Lookups::M;
    }

    // Convert speed to step time
    double steptime = 1.0 / speed;

    // If enough time has passed since the last step, perform the next step
    if (current_micros - last_step_time >= steptime) {
        bool dir = delta_steps > 0; // Determine the direction of the step
        step(dir); // Perform the step
        last_step_time = current_micros; // Update the last step time
        if (dir) {
            delta_steps -= 1; // Decrease the step count if moving forward
        } else {
            delta_steps += 1; // Increase the step count if moving backward
        }
        iterations += 1; // Increment the iteration count
    }
}
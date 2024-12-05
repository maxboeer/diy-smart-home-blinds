//
// Created by maxim on 05.12.2024.
//

#include "Blind.h"

Blind::Blind(int dir_pin, int step_pin, int position, int top_steps, int bottom_steps, const String& BlindID){
    this->dir_pin = dir_pin;
    this->step_pin = step_pin;
    this->position = position;
    this->top_steps = top_steps;
    this->bottom_steps = bottom_steps;
    this->sinricBlind = SinricHandler::getBlind(BlindID);

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

    // Parameters
    double polynomial_percentage = 0.15; // Percentage of total steps for polynomial
    double inflection_shift_percentage = 0.1; // Shift inflection points symmetrically
    double curvature = 3.0; // Curvature factor
    double max_speed = 1.0 / steptime_low; // Maximum speed

    // Calculate total steps and polynomial length
    double total_steps = abs(delta_steps) + iterations;
    double polynomial_length = total_steps * polynomial_percentage;
    double inflection_shift = polynomial_length * inflection_shift_percentage;

    // Calculate normalized time (0 to 1)
    double t = double(iterations) / total_steps;
    double speed = 0.0;

    // Acceleration phase (cubic polynomial)
    if (t <= (polynomial_length + inflection_shift) / total_steps) {
        double t_accel = t / ((polynomial_length + inflection_shift) / total_steps);
        speed = max_speed * (curvature * t_accel * t_accel - (curvature - 1) * t_accel * t_accel * t_accel);
    }
        // Constant speed phase
    else if (t > (polynomial_length + inflection_shift) / total_steps && t < (1 - (polynomial_length + inflection_shift) / total_steps)) {
        speed = max_speed;
    }
        // Deceleration phase (cubic polynomial)
    else if (t >= (1 - (polynomial_length + inflection_shift) / total_steps)) {
        double t_decel = (1 - t) / ((polynomial_length + inflection_shift) / total_steps);
        speed = max_speed * (curvature * t_decel * t_decel - (curvature - 1) * t_decel * t_decel * t_decel);
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
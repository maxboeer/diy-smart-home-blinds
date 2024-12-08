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
    double L_accel = 1000; // Total length of the acceleration phase
    double L_decel = 1000; // Total length of the deceleration phase
    double M = 1.0 / steptime_low; // Maximum speed (y-value)
    double c_accel = 3.0; // Curvature for acceleration
    double c_decel = 3.0; // Curvature for deceleration
    double t_w_accel = 0.5; // Midpoint of the acceleration phase (percentage of L_accel)
    double t_w_decel = 0.5; // Midpoint of the deceleration phase (percentage of L_decel)

    // Calculate normalized time (0 to 1)
    double x = double(iterations);
    double speed = 0.0;

    // Acceleration phase
    if (x <= L_accel) {
        speed = M * pow(x / (t_w_accel * L_accel), c_accel) / (pow(x / (t_w_accel * L_accel), c_accel) + pow((L_accel - x) / ((1 - t_w_accel) * L_accel), c_accel));
    }
        // Deceleration phase
    else if (x >= (abs(delta_steps) - L_decel)) {
        double x_decel = abs(delta_steps) - x;
        speed = M * pow(x_decel / (t_w_decel * L_decel), c_decel) / (pow(x_decel / (t_w_decel * L_decel), c_decel) + pow((L_decel - x_decel) / ((1 - t_w_decel) * L_decel), c_decel));
    }
        // Constant speed phase
    else {
        speed = M;
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
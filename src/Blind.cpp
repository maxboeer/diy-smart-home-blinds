//
// Created by maxim on 05.12.2024.
//

#include "Blind.h"

int Blind::blindCount = 0;

Blind::Blind(int dir_pin, int step_pin, int position, int top_steps, int bottom_steps, const String& BlindID, const Lookups::accelLookup<(unsigned int)Lookups::L_accel>* accel_lookup, const Lookups::decelLookup<(unsigned int)Lookups::L_decel>* decel_lookup) {
    this->dir_pin = dir_pin;
    this->step_pin = step_pin;
    this->position = position;
    this->target_positions.push(position);
    this->last_target_position = position;
    this->top_steps = top_steps;
    this->bottom_steps = bottom_steps;
    this->sinricBlind = SinricHandler::getBlind(BlindID);
    this->accel_lookup = accel_lookup;
    this->decel_lookup = decel_lookup;
    this->iterations = 0;
    this->last_step_time = micros();
    for (unsigned int i = 0; i < accel_lookup->length; ++i) {
        Serial.println("accel_lookup[" + String(i) + "] = " + String(accel_lookup->arr[i]));
    }
    Serial.println("Steptime: " + String(1 / accel_lookup->arr[0]));
    this->steptime = 1 / accel_lookup->arr[0];
    this->id = blindCount++;

    pinMode(dir_pin, OUTPUT);
    pinMode(step_pin, OUTPUT);
    digitalWrite(step_pin, 0);
}

void Blind::step(bool dir){
    dir ? ++position : --position;
    digitalWrite(this->dir_pin, dir);
    digitalWrite(this->step_pin, HIGH);
    digitalWrite(this->step_pin, LOW);
}

void Blind::doTick() {
    //TODO: Handle changes in target position during movement (maybe target position queue)
    //TODO: check for target position out of bounds
    unsigned long current_micros = micros(); // Get the current time in microseconds

    // If enough time has passed since the last step, perform the next step
    if (current_micros - last_step_time >= steptime) {
//        Serial.println("Steptime: " + String(steptime, 20));
        // Calculate the number of steps to perform
        int delta_steps = target_positions.front() - position;

        // Perform the step
        {
            bool dir = delta_steps > 0; // Determine the direction of the step
            step(dir); // Perform the step
            last_step_time = current_micros; // Update the last step time
            ++iterations; // Increment the iteration count
        }

        // Calculate the step time
        {
            int total_delta_steps = abs(target_positions.front() - last_target_position); // Calculate the total number of steps to the target position

            // Determine the number of steps for acceleration and deceleration
            int accel_steps = min(total_delta_steps / 2, accel_lookup->length);
            int decel_steps = min(total_delta_steps / 2, decel_lookup->length);

            double x = double(iterations);
            double speed = 0.0;

            // Acceleration phase
            if (x < accel_steps) {
                speed = accel_lookup->arr[int(x)];
            }
            // Deceleration phase
            else if (x > (total_delta_steps - decel_steps)) {
                double x_decel = total_delta_steps - x;
                speed = decel_lookup->arr[int(x_decel)];
            }
            // Constant speed phase
            else {
                speed = Lookups::M;
            }

            // Convert speed to step time
            steptime = 1.0 / speed;
        }
    }
}
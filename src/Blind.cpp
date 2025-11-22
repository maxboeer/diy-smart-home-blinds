#include "Blind.h"

#include <algorithm>

int Blind::blindCount = 0;

namespace {
    constexpr MotionPlanner::Limits DEFAULT_LIMITS{
            2000.0,   // vmax steps / s
            8000.0,   // amax steps / s^2
            40000.0,  // jmax steps / s^3
            120000.0  // smax steps / s^4
    };
}

Blind::Blind(int dir_pin, int step_pin, int position, int top_steps, int bottom_steps, const String& BlindID) :
        dir_pin(dir_pin),
        step_pin(step_pin),
        planner(double(position), DEFAULT_LIMITS) {
    this->position = position;
    this->last_target_position = position;
    this->top_steps = top_steps;
    this->bottom_steps = bottom_steps;
    this->sinricBlind = SinricHandler::getBlind(BlindID);
    this->stepAccumulator = 0.0;
    this->lastUpdateMicros = 0;
    this->id = blindCount++;

    pinMode(dir_pin, OUTPUT);
    pinMode(step_pin, OUTPUT);
    digitalWrite(step_pin, 0);
}

bool Blind::canStep(bool dir) const {
    if (dir) return position < top_steps;
    return position > bottom_steps;
}

void Blind::step(bool dir) {
    if (!canStep(dir)) return;
    dir ? ++position : --position;
    digitalWrite(this->dir_pin, dir);
    digitalWrite(this->step_pin, HIGH);
    digitalWrite(this->step_pin, LOW);
}

bool Blind::wantsMotion() const {
    return !target_positions.empty() || planner.hasWork();
}

void Blind::doTick() {
    unsigned long nowMicros = micros();
    if (lastUpdateMicros == 0) {
        lastUpdateMicros = nowMicros;
        return;
    }

    double dt = double(nowMicros - lastUpdateMicros) * 1e-6;
    lastUpdateMicros = nowMicros;

    if (dt <= 0.0) return;

    int target = 0;
    bool hasTarget = false;
    // Pull the latest target (always jump to newest command)
    if (!target_positions.empty()) {
        target = target_positions.back();
        target = std::min(std::max(target, bottom_steps), top_steps);
        hasTarget = true;

        // Re-issue a correction if there is a new target or we were idle.
        if (target != int(planner.getUltimateTarget()) || !planner.hasWork()) {
            planner.setPosition(double(position));
            planner.addTarget(double(target));
            last_target_position = target;
        }
    }

    if (!planner.hasWork()) {
        if (hasTarget && position == target) {
            while (!target_positions.empty()) target_positions.pop();
            last_target_position = target;
        }
        stepAccumulator = 0.0;
        return;
    }

    double velocity = 0.0;
    double lambda = 1.0;
    planner.update(dt, velocity, lambda);
    (void) lambda;

    const double delta = velocity * dt;
    stepAccumulator += delta;

    while (stepAccumulator >= 1.0) {
        if (!canStep(true)) {
            // Abort motion if we somehow try to exceed the upper bound.
            stepAccumulator = 0.0;
            planner.clearAll();
            while (!target_positions.empty()) target_positions.pop();
            last_target_position = position;
            return;
        }
        step(true);
        stepAccumulator -= 1.0;
    }
    while (stepAccumulator <= -1.0) {
        if (!canStep(false)) {
            // Abort motion if we somehow try to exceed the lower bound.
            stepAccumulator = 0.0;
            planner.clearAll();
            while (!target_positions.empty()) target_positions.pop();
            last_target_position = position;
            return;
        }
        step(false);
        stepAccumulator += 1.0;
    }

    // If a target was reached, clear the queue and resync state.
    if (!planner.hasWork() && !target_positions.empty()) {
        if (position == target_positions.back()) {
            while (!target_positions.empty()) target_positions.pop();
            stepAccumulator = 0.0;
            planner.setPosition(double(position));
        }
    }
}

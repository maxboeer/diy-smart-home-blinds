//
// Bump-function based motion planner with v/a/j/s limits.
//

#include "MotionPlanner.h"

#include <cmath>
#include <algorithm>

static constexpr double MIN_DURATION_SEC = 0.02; // avoid unrealistically short pulses
static constexpr double DISABLE_LIMIT = 1e6;     // treat larger limits as disabled

constexpr double MotionPlanner::fRaw(double x) {
    if (x <= 0.0) return 0.0;
    return gcem::exp(-1.0 / x);
}

constexpr double MotionPlanner::gRaw(double x) {
    if (x <= 0.0) return 0.0;
    if (x >= 1.0) return 1.0;
    const double fx = fRaw(x);
    const double f1 = fRaw(1.0 - x);
    return fx / (fx + f1);
}

constexpr double MotionPlanner::gPrimeRaw(double x) {
    if (x <= 0.0 || x >= 1.0) return 0.0;
    constexpr double h = 1e-4;
    return (gRaw(x + h) - gRaw(x - h)) / (2.0 * h);
}

constexpr double MotionPlanner::gSecondRaw(double x) {
    constexpr double h = 5e-3;
    if (x <= h || x >= 1.0 - h) return 0.0;
    return (gRaw(x + h) - 2.0 * gRaw(x) + gRaw(x - h)) / (h * h);
}

constexpr double MotionPlanner::gThirdRaw(double x) {
    constexpr double h = 3e-3;
    if (x <= h || x >= 1.0 - h) return 0.0;
    return (gSecondRaw(x + h) - gSecondRaw(x - h)) / (2.0 * h);
}

constexpr double MotionPlanner::gFourthRaw(double x) {
    constexpr double h = 4e-3;
    if (x <= h || x >= 1.0 - h) return 0.0;
    return (gSecondRaw(x + h) - 2.0 * gSecondRaw(x) + gSecondRaw(x - h)) / (h * h);
}

const std::array<double, MotionPlanner::LUT_SAMPLES> MotionPlanner::G_LUT = MotionPlanner::buildLut<MotionPlanner::gRaw>();
const std::array<double, MotionPlanner::LUT_SAMPLES> MotionPlanner::G1_LUT = MotionPlanner::buildLut<MotionPlanner::gPrimeRaw>();
const std::array<double, MotionPlanner::LUT_SAMPLES> MotionPlanner::G2_LUT = MotionPlanner::buildLut<MotionPlanner::gSecondRaw>();
const std::array<double, MotionPlanner::LUT_SAMPLES> MotionPlanner::G3_LUT = MotionPlanner::buildLut<MotionPlanner::gThirdRaw>();
const std::array<double, MotionPlanner::LUT_SAMPLES> MotionPlanner::G4_LUT = MotionPlanner::buildLut<MotionPlanner::gFourthRaw>();

const double MotionPlanner::G1_MAX = MotionPlanner::maxAbs(MotionPlanner::G1_LUT);
const double MotionPlanner::G2_MAX = MotionPlanner::maxAbs(MotionPlanner::G2_LUT);
const double MotionPlanner::G3_MAX = MotionPlanner::maxAbs(MotionPlanner::G3_LUT);
const double MotionPlanner::G4_MAX = MotionPlanner::maxAbs(MotionPlanner::G4_LUT);

MotionPlanner::MotionPlanner(double initialPos, const Limits &limits) : limits(limits), pos(initialPos), ultimateTarget(initialPos) {}

void MotionPlanner::setLimits(const Limits &newLimits) {
    limits = newLimits;
}

void MotionPlanner::setPosition(double newPos) {
    pos = newPos;
}

double MotionPlanner::getUltimateTarget() const {
    return ultimateTarget;
}

bool MotionPlanner::hasWork() const {
    return !corrections.empty();
}

void MotionPlanner::clearAll() {
    corrections.clear();
    ultimateTarget = pos;
}

void MotionPlanner::addTarget(double target) {
    const double anchor = corrections.empty() ? pos : ultimateTarget;
    const double delta = target - anchor;
    const double dist = std::fabs(delta);
    if (dist < 1e-6) {
        ultimateTarget = target;
        return;
    }

    double durations[4];
    size_t durationCount = 0;

    if (std::isfinite(limits.v) && limits.v > 0.0 && limits.v < DISABLE_LIMIT) {
        durations[durationCount++] = dist * G1_MAX / limits.v;
    }
    if (std::isfinite(limits.a) && limits.a > 0.0 && limits.a < DISABLE_LIMIT) {
        durations[durationCount++] = std::sqrt(dist * G2_MAX / limits.a);
    }
    if (std::isfinite(limits.j) && limits.j > 0.0 && limits.j < DISABLE_LIMIT) {
        durations[durationCount++] = std::cbrt(dist * G3_MAX / limits.j);
    }
    if (std::isfinite(limits.s) && limits.s > 0.0 && limits.s < DISABLE_LIMIT) {
        durations[durationCount++] = std::pow(dist * G4_MAX / limits.s, 0.25);
    }

    double duration = durationCount ? *std::max_element(durations, durations + durationCount) : dist / limits.v;
    if (!std::isfinite(duration) || duration <= 0.0) duration = 0.3;
    duration = std::max(duration, MIN_DURATION_SEC);
    const double baseRate = 1.0 / duration;

    corrections.push_back({0.0, baseRate, delta});
    ultimateTarget = target;
}

bool MotionPlanner::update(double dt, double &velocity, double &lambda) {
    velocity = 0.0;
    lambda = 1.0;
    if (dt <= 0.0 || corrections.empty()) return false;

    double vRaw = 0.0, aRaw = 0.0, jRaw = 0.0, sRaw = 0.0;
    computeRawDerivatives(vRaw, aRaw, jRaw, sRaw);

    const double factors[4] = {
            factorFromNorm(std::fabs(vRaw), limits.v),
            factorFromNorm(std::fabs(aRaw), limits.a),
            factorFromNorm(std::fabs(jRaw), limits.j),
            factorFromNorm(std::fabs(sRaw), limits.s)
    };

    lambda = softMinFactors(factors, 4, -4.0);
    velocity = vRaw * lambda;

    const double phaseAdvance = dt * lambda;
    for (auto &c: corrections) {
        if (c.s >= 1.0) continue;
        c.s += phaseAdvance * c.baseRate;
        if (c.s > 1.0) c.s = 1.0;
    }

    pos += velocity * dt;
    pruneCorrections();
    return true;
}

// --- helpers --------------------------------------------------------------
double MotionPlanner::g(double x) {
    if (x <= 0.0) return 0.0;
    if (x >= 1.0) return 1.0;
    return sampleLut(G_LUT.data(), LUT_SAMPLES, x);
}

double MotionPlanner::gPrime(double x) {
    if (x <= 0.0 || x >= 1.0) return 0.0;
    return sampleLut(G1_LUT.data(), LUT_SAMPLES, x);
}

double MotionPlanner::gSecond(double x) {
    const double h = 1.0 / double(LUT_SAMPLES);
    if (x <= h || x >= 1.0 - h) return 0.0;
    return sampleLut(G2_LUT.data(), LUT_SAMPLES, x);
}

double MotionPlanner::gThird(double x) {
    const double h = 1.0 / double(LUT_SAMPLES);
    if (x <= h || x >= 1.0 - h) return 0.0;
    return sampleLut(G3_LUT.data(), LUT_SAMPLES, x);
}

double MotionPlanner::gFourth(double x) {
    const double h = 1.0 / double(LUT_SAMPLES);
    if (x <= h || x >= 1.0 - h) return 0.0;
    return sampleLut(G4_LUT.data(), LUT_SAMPLES, x);
}

double MotionPlanner::factorFromNorm(double norm, double limit) {
    if (!std::isfinite(limit) || limit <= 0.0 || limit >= DISABLE_LIMIT) return 1.0;
    if (norm == 0.0) return 1.0;
    return limit / std::sqrt(norm * norm + limit * limit);
}

double MotionPlanner::softMinFactors(const double *factors, size_t count, double k) {
    if (count == 0) return 1.0;
    double sum = 0.0;
    for (size_t i = 0; i < count; ++i) {
        const double f = std::max(1e-6, std::min(1.0, factors[i]));
        sum += std::pow(f, k);
    }
    return std::pow(sum / double(count), 1.0 / k);
}

double MotionPlanner::sampleLut(const double *arr, size_t count, double x) {
    if (count == 0) return 0.0;
    const double xi = std::min(std::max(x, 0.0), 1.0) * double(count - 1);
    const size_t idx = static_cast<size_t>(xi);
    const double t = xi - double(idx);
    const double a = arr[idx];
    const double b = arr[std::min(idx + 1, count - 1)];
    return a + (b - a) * t;
}

void MotionPlanner::computeRawDerivatives(double &vRaw, double &aRaw, double &jRaw, double &sRaw) const {
    for (const auto &c: corrections) {
        if (c.s <= 0.0 || c.s >= 1.0) continue;
        const double br = c.baseRate;
        const double br2 = br * br;
        const double br3 = br2 * br;
        const double br4 = br3 * br;

        const double g1 = gPrime(c.s);
        const double g2 = gSecond(c.s);
        const double g3 = gThird(c.s);
        const double g4 = gFourth(c.s);

        vRaw += c.delta * g1 * br;
        aRaw += c.delta * g2 * br2;
        jRaw += c.delta * g3 * br3;
        sRaw += c.delta * g4 * br4;
    }
}

void MotionPlanner::pruneCorrections() {
    corrections.erase(std::remove_if(corrections.begin(), corrections.end(), [](const Correction &c) {
        return c.s >= 1.0;
    }), corrections.end());
}

//
// Bump-function based motion planner with v/a/j/s limits.
//

#ifndef DIY_SMART_HOME_BLINDS_MOTIONPLANNER_H
#define DIY_SMART_HOME_BLINDS_MOTIONPLANNER_H

#include <vector>
#include <cstddef>
#include <array>
#include "gcem.hpp"

class MotionPlanner {
public:
    struct Limits {
        double v; // steps / s
        double a; // steps / s^2
        double j; // steps / s^3
        double s; // steps / s^4
    };

    MotionPlanner(double initialPos, const Limits &limits);

    void setLimits(const Limits &limits);
    void setPosition(double pos);
    void addTarget(double target);
    bool update(double dt, double &velocity, double &lambda);
    double getUltimateTarget() const;
    bool hasWork() const;
    void clearAll();

private:
    static constexpr size_t LUT_SAMPLES = 256;

    struct Correction {
        double s;
        double baseRate;
        double delta;
    };

    std::vector<Correction> corrections;
    Limits limits;
    double pos;
    double ultimateTarget;

    static constexpr double fRaw(double x);
    static constexpr double gRaw(double x);
    static constexpr double gPrimeRaw(double x);
    static constexpr double gSecondRaw(double x);
    static constexpr double gThirdRaw(double x);
    static constexpr double gFourthRaw(double x);

    static double g(double x);
    static double gPrime(double x);
    static double gSecond(double x);
    static double gThird(double x);
    static double gFourth(double x);

    static double factorFromNorm(double norm, double limit);
    static double softMinFactors(const double *factors, size_t count, double k);

    void computeRawDerivatives(double &vRaw, double &aRaw, double &jRaw, double &sRaw) const;
    void pruneCorrections();
    static double sampleLut(const double *arr, size_t count, double x);

    template<double (*Fn)(double)>
    static constexpr std::array<double, LUT_SAMPLES> buildLut() {
        std::array<double, LUT_SAMPLES> arr{};
        for (size_t i = 0; i < LUT_SAMPLES; ++i) {
            const double x = (LUT_SAMPLES == 1) ? 0.0 : (double(i) / double(LUT_SAMPLES - 1));
            arr[i] = Fn(x);
        }
        return arr;
    }

    static constexpr double maxAbs(const std::array<double, LUT_SAMPLES> &arr) {
        double m = 0.0;
        for (size_t i = 0; i < LUT_SAMPLES; ++i) {
            const double v = arr[i] < 0.0 ? -arr[i] : arr[i];
            if (v > m) m = v;
        }
        return m;
    }

    static const double G1_MAX;
    static const double G2_MAX;
    static const double G3_MAX;
    static const double G4_MAX;
    static const std::array<double, LUT_SAMPLES> G_LUT;
    static const std::array<double, LUT_SAMPLES> G1_LUT;
    static const std::array<double, LUT_SAMPLES> G2_LUT;
    static const std::array<double, LUT_SAMPLES> G3_LUT;
    static const std::array<double, LUT_SAMPLES> G4_LUT;
};

#endif //DIY_SMART_HOME_BLINDS_MOTIONPLANNER_H

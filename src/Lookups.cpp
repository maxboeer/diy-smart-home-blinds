#include "gcem.hpp"

namespace Lookups {
    //    double interpolation_factor = log10(steptime_low/steptime_high) / (interpolation_length * log10(double(1)/double(2)));
    static constexpr double min_speed = 1.0 / 100000; // in steps per microsecond / steptime_high = 100 milliseconds
    static constexpr double M = 1.0 / 500; // Assuming steptime_low = 100 microseconds
    static constexpr double c_accel = 1.2;
    static constexpr double c_decel = 1.2;
    static constexpr double t_w_accel = 0.5;
    static constexpr double t_w_decel = 0.5;


    template<int N>
        struct accelLookup {
            constexpr accelLookup() : arr(), length(N) {
                for (auto i = 0; i < N; ++i)
                    arr[i] = min_speed + (M - min_speed) * gcem::pow(i / (t_w_accel * N), c_accel) / (gcem::pow(i / (t_w_accel * N), c_accel) + gcem::pow((N - i) / ((1 - t_w_accel) * N), c_accel));
            }
            double arr[N];
            int length = N;
        };

    template<int N>
        struct decelLookup {
            constexpr decelLookup() : arr(), length(N) {
                for (auto i = 0; i < N; ++i)
                    arr[i] = min_speed + (M - min_speed) * gcem::pow(i / (t_w_decel * N), c_decel) / (gcem::pow(i / (t_w_decel * N), c_decel) + gcem::pow((N - i) / ((1 - t_w_decel) * N), c_decel));
            }
            double arr[N];
            int length = N;
        };

    static constexpr double L_accel = 500;
    static constexpr double L_decel = 500;
    static constexpr Lookups::accelLookup<(unsigned int)L_accel> accel_lookup = Lookups::accelLookup<(unsigned int)L_accel>();
    static constexpr Lookups::decelLookup<(unsigned int)L_decel> decel_lookup = Lookups::decelLookup<(unsigned int)L_decel>();
}
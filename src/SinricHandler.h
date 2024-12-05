//
// Created by maxim on 05.12.2024.
//

#ifndef DIY_SMART_HOME_BLINDS_SINRICHANDLER_H
#define DIY_SMART_HOME_BLINDS_SINRICHANDLER_H

#include <Arduino.h>
#include <vector>
#include <memory>

namespace SINRICPRO_NAMESPACE {
    class SinricProBlinds;
}
class Blind;

class SinricHandler {
public:
    SinricHandler(std::vector<Blind*> &blinds, const String &app_key, const String &app_secret);
    static void handle();
    static SINRICPRO_NAMESPACE::SinricProBlinds* getBlind(const String &deviceId);
private:
    static bool onPowerState(const String &deviceId, bool &state);
    static bool onRangeValue(const String &deviceId, int &position);
    static bool onAdjustRangeValue(const String &deviceId, int &positionDelta);
    static std::vector<Blind*> &blinds;
};


#endif //DIY_SMART_HOME_BLINDS_SINRICHANDLER_H

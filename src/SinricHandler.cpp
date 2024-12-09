#include "SinricPro.h"
#include "SinricProBlinds.h"

#include "SinricHandler.h"
#include "Blind.h"

std::vector<Blind*> &(SinricHandler::blinds) = *(new std::vector<Blind*>);

 SinricHandler::SinricHandler(std::vector<Blind*> &blinds, const String &app_key, const String &app_secret){
    SinricHandler::blinds = blinds;
    // Register callbacks for blinds
    for(auto blind : blinds){
        blind->sinricBlind->onPowerState(SinricHandler::onPowerState);
        blind->sinricBlind->onRangeValue(SinricHandler::onRangeValue);
        blind->sinricBlind->onAdjustRangeValue(SinricHandler::onAdjustRangeValue);
    }

    // Setup SinricPro connection
    SinricPro.onConnected([](){ Serial.printf("[SINRIC]: Connected to SinricPro\r\n"); });
    SinricPro.onDisconnected([](){ Serial.printf("[SINRIC]: Disconnected from SinricPro\r\n"); });
    SinricPro.begin(app_key, app_secret);
}

void SinricHandler::handle() {
    SinricPro.handle();
}

bool SinricHandler::onPowerState(const String &deviceId, bool &state) {
    // Serial.printf("[SINRIC]: Device %s power turned %s \r\n", deviceId.c_str(), state?"on":"off");
    return true; // request handled properly
}

bool SinricHandler::onRangeValue(const String &deviceId, int &position) {
    for (auto blind : blinds) {
        if (blind->sinricBlind->getDeviceId() == deviceId) {
            blind->target_position = map(position, 0, 100, blind->bottom_steps, blind->top_steps);
            EEPROM::storeUInt("steps_" + String(blind->id), blind->target_position);
            break;
        }
    }

    //Serial.printf("[SINRIC]: Device %s set position to %d\r\n", deviceId.c_str(), position);
    return true; // request handled properly
}

bool SinricHandler::onAdjustRangeValue(const String &deviceId, int &positionDelta) {
    Blind* blind = nullptr;

    for (Blind* element : blinds) {
        if (element->sinricBlind->getDeviceId() == deviceId) {
            blind = element;
            break;
        }
    }

    blind->target_position = blind->position + map(positionDelta, 0, 100, blind->bottom_steps, blind->top_steps);
    EEPROM::storeUInt("steps_" + String(blind->id), blind->target_position);

    //Serial.printf("[SINRIC]: Device %s position changed about %i to %i", deviceId.c_str(), positionDelta, blind->target_position);
    positionDelta = blind->target_position; // calculate and return absolute position
    return true; // request handled properly
}

SinricProBlinds *SinricHandler::getBlind(const String &deviceId) {
    // TODO: Check if new SinricProBlinds(deviceId) is the same as SinricPro[deviceId];
    // SinrricPro[deviceId];
    return new SinricProBlinds (deviceId);
}

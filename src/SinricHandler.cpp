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
     Serial.printf("[SINRIC]: Device %s power turned %s \r\n", deviceId.c_str(), state?"on":"off");
    return true; // request handled properly
}

bool SinricHandler::onRangeValue(const String &deviceId, int &position) {
    Serial.println("onRangeValue");

    for (auto blind : blinds) {
        if (blind->sinricBlind->getDeviceId() == deviceId) {
            blind->target_positions.push(map(position, 0, 100, blind->bottom_steps, blind->top_steps));
            EEPROM::storeUInt("steps_" + String(blind->id), blind->target_positions.front());
            break;
        }
    }

    Serial.printf("[SINRIC]: Device %s set position to %d\r\n", deviceId.c_str(), position);
    return true; // request handled properly
}

//could lead to issues with queue, but actually never used
bool SinricHandler::onAdjustRangeValue(const String &deviceId, int &positionDelta) {
    Serial.println("onAdjustRangeValue");
    Blind* blind = nullptr;

    for (Blind* element : blinds) {
        if (element->sinricBlind->getDeviceId() == deviceId) {
            blind = element;
            break;
        }
    }

    blind->target_positions.push(blind->position + map(positionDelta, 0, 100, blind->bottom_steps, blind->top_steps));
    EEPROM::storeUInt("steps_" + String(blind->id), blind->target_positions.front());

    Serial.printf("[SINRIC]: Device %s position changed about %i to %i", deviceId.c_str(), positionDelta, blind->target_positions.back());
    positionDelta = blind->target_positions.back(); // calculate and return absolute position
    return true; // request handled properly
}

SinricProBlinds *SinricHandler::getBlind(const String &deviceId) {
    SinricProBlinds& blind = SinricPro[deviceId];
    return &blind;
//    return new SinricProBlinds (deviceId);
}

//
// Created by maxim on 05.12.2024.
//

#ifndef DIY_SMART_HOME_BLINDS_WIFIMANAGER_H
#define DIY_SMART_HOME_BLINDS_WIFIMANAGER_H

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#endif

class WifiManager {
public:
    static void setup(const String &ssid, const String &passphrase, unsigned long reconnectInterval);
private:
    static void restart();
    static void reconnect();
    static void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
    static unsigned long reconnectInterval;
};


#endif //DIY_SMART_HOME_BLINDS_WIFIMANAGER_H

//
// Created by maxim on 05.12.2024.
//

#include "WifiManager.h"
#include "BlindManager.h"

unsigned long WifiManager::reconnectInterval = 1000*1000*15;

void WifiManager::setup(const String &ssid, const String &passphrase, const unsigned long reconnectInterval) {
    WifiManager::reconnectInterval = reconnectInterval;

    WiFi.disconnect(true);
    delay(1000);
    WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.begin(ssid, passphrase);
    Serial.printf("\r\n[Wifi]: Connecting");

    reconnect();

    IPAddress localIP = WiFi.localIP();
    Serial.printf("connected!\r\n[WiFi]: IP-Address is %d.%d.%d.%d\r\n", localIP[0], localIP[1], localIP[2], localIP[3]);
}

void WifiManager::WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    reconnect();
}

void WifiManager::reconnect() {
    while (WiFi.status() != WL_CONNECTED) {
        unsigned long current_micros = micros();
        static unsigned long previousmicrosWifi = current_micros;
        static unsigned long previousmicrosDot = current_micros;
        static unsigned int reconnect_counter = 0;

        // Check if the reconnect interval has passed
        if (current_micros - previousmicrosWifi >= reconnectInterval){
            if(reconnect_counter >= 10)
                restart(); // Restart if too many reconnect attempts

            Serial.println("");
            Serial.print("Reconnecting");
            WiFi.disconnect();
            WiFi.reconnect();
            reconnect_counter += 1;
            previousmicrosWifi = current_micros;
        }

        // Print a dot every 250ms to indicate reconnecting
        if (current_micros - previousmicrosDot >= 250000){
            Serial.printf(".");
            previousmicrosDot = current_micros;
        }
    }
}

void WifiManager::restart() {
    Serial.println("restarting in 5 Seconds");
//    // Save the current step positions to preferences
//    if(preferences.getUInt("steps_right") != step_position_right)
//        preferences.putUInt("steps_right", step_position_right);
//    if(preferences.getUInt("steps_left") != step_position_left)
//        preferences.putUInt("steps_left", step_position_left);
//    preferences.end();
    delay(5000); // Wait for 5 seconds before restarting
    ESP.restart();
}

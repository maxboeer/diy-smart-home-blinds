//
// Created by maxim on 07.01.2026.
//

#include "OTAManager.h"
#include "WifiManager.h"
#include <ArduinoOTA.h>
#include "esp_system.h"

void OTAManager::setup(const String& hostname, const uint16_t& port, const String& password) {
    ArduinoOTA.setPort(port);
    ArduinoOTA.setHostname(hostname.c_str());
    ArduinoOTA.setPassword(password.c_str());

    ArduinoOTA
            .onStart([]() {
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH) {
                    type = "sketch";
                } else {  // U_SPIFFS
                    type = "filesystem";
                }

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                Serial.println("[OTA]: Updating " + type);
            })
            .onEnd([]() {
                Serial.println("\n[OTA]: Update finished!");
                Serial.println("[SYSTEM]: Restarting now...");
                Serial.flush();
                // Disable WiFi reconnect to prevent event loop during restart
                WifiManager::disableReconnect();
                delay(500);

                esp_restart();
            })
            .onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("[OTA]: Progress: %u%%\r", (progress / (total / 100)));
            })
            .onError([](ota_error_t error) {
                Serial.printf("[OTA]: Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) {
                    Serial.println("Auth Failed");
                } else if (error == OTA_BEGIN_ERROR) {
                    Serial.println("Begin Failed");
                } else if (error == OTA_CONNECT_ERROR) {
                    Serial.println("Connect Failed");
                } else if (error == OTA_RECEIVE_ERROR) {
                    Serial.println("Receive Failed");
                } else if (error == OTA_END_ERROR) {
                    Serial.println("End Failed");
                }
            });

    ArduinoOTA.begin();

    Serial.printf("[OTA]: Hostname: %s\r\n", hostname.c_str());
    Serial.printf("[OTA]: Port: %d\r\n", port);
    Serial.printf("[OTA]: Password: %s\r\n", password.length() > 0 ? "***SET***" : "NOT SET");
    Serial.println("[OTA]: OTA ready!\r\n");
}

void OTAManager::handle() {
    ArduinoOTA.handle();
}

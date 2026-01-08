#include <Arduino.h>
#define LED_BUILTIN 2

#include "esp32-hal-cpu.h"

#include <Preferences.h>

Preferences preferences;

#include "secrets.h"
#include "WifiManager.h"
#include "BlindManager.h"
#include "OTAManager.h"
#include "MQTTManager.h"

// Main setup function
BlindManager* blindManager;
MQTTManager* mqttManager;

void setup() {
    setCpuFrequencyMhz(240); // Set CPU clock to 240MHz

    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200); Serial.printf("\r\n\r\n");
    Serial.println("[SYSTEM]: Starting up...");

    EEPROM::init("blinds");

    WifiManager::setup(secrets.wifi.ssid, secrets.wifi.pass, 1000*1000*15, secrets.ota.hostname);
    OTAManager::setup(secrets.ota.hostname, secrets.ota.port, secrets.ota.password);

    blindManager = new BlindManager(32, 33);
    blindManager->addBlind(19, 18, (int)EEPROM::readUInt("steps_0"), 25500, 0, "blind_left");
    blindManager->addBlind(26, 27, (int)EEPROM::readUInt("steps_1"), 25500, 0, "blind_right");
    mqttManager = new MQTTManager(secrets.mqtt.broker_ip, secrets.mqtt.user, secrets.mqtt.password);

    delay(500);
}

// Main loop function
void loop() {
    OTAManager::handle();
    mqttManager->handle();
    blindManager->handle();
}
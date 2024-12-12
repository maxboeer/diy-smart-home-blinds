#include <Arduino.h>
#define LED_BUILTIN 2

#include "esp32-hal-cpu.h"

#include <Preferences.h>

Preferences preferences;

#include "secrets.h"
#include "WifiManager.h"
#include "BlindManager.h"
#include "SinricHandler.h"

// Main setup function
SinricHandler* sinric;
BlindManager* blindManager;

void setup() {
    setCpuFrequencyMhz(240); // Set CPU clock to 240MHz

    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(secrets.sinric.baud_rate); Serial.printf("\r\n\r\n");
    Serial.println("[SYSTEM]: Starting up...");

    EEPROM::init("blinds");

    WifiManager::setup(secrets.wifi.ssid, secrets.wifi.pass, 1000*1000*15);
    blindManager = new BlindManager(32, 33);
    blindManager->addBlind(19, 18, (int)EEPROM::readUInt("steps_0"), 25500, 0, secrets.sinric.right_blinds_id);
    blindManager->addBlind(26, 27, (int)EEPROM::readUInt("steps_1"), 25500, 0, secrets.sinric.left_blinds_id);
    sinric = new SinricHandler(blindManager->blinds, secrets.sinric.app_key, secrets.sinric.app_secret);

    delay(500);
}

// Main loop function
void loop() {
    //SinricPro.handle();
    sinric->handle();
    //step();
    blindManager->handle();
}
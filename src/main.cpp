#include <Arduino.h>
#define LED_BUILTIN 2

#include "esp32-hal-cpu.h"

#include <Preferences.h>

Preferences preferences;

#include "secrets.h"
#include "WifiManager.h"
#include "BlindManager.h"
#include "SinricHandler.h"

//int dir_pin1 = 23;
//int step_pin1 = 22;
//int dir_pin2 = 18;
//int step_pin2 = 19;
//const int powerOnPin = 17;
//const int powerControlPin = 16;
//const int top_steps = 17650;
//double interpolation_factorOLD = log10(steptime_low/steptime_high) / (interpolation_length * log10(double(1)/double(2)));

//const unsigned long reconnectInterval = 1000*1000*15;
//unsigned int reconnect_counter = 0;

// Main setup function
SinricHandler* sinric;
BlindManager* blindManager;

void setup() {
    setCpuFrequencyMhz(240); // Set CPU clock to 240MHz

    Serial.begin(secrets.sinric.baud_rate); Serial.printf("\r\n\r\n");
    preferences.begin("rollos", false);

    //setupWiFi();
    //Sinric Setup
    //setupBlinds();
    WifiManager::setup(secrets.wifi.ssid, secrets.wifi.pass, 1000*1000*15);
    blindManager = new BlindManager(17, 16);
    blindManager->addBlind(23, 22, 0, 17650, 0, secrets.sinric.right_blinds_id);
    blindManager->addBlind(18, 19, 0, 17650, 0, secrets.sinric.left_blinds_id);
    sinric = new SinricHandler(blindManager->blinds, secrets.sinric.app_key, secrets.sinric.app_secret);

    delay(500);
}

// Main loop function
void loop() {
    //SinricPro.handle();
    sinric->handle();
    //step();
    blindManager->handle();

    // Print WiFi signal strength every 30 seconds
//    if(WiFi.status() == WL_CONNECTED && current_micros - rssi_micros_prev >= 30000000){
//        Serial.print("Connected Network Signal Strength (RSSI): ");
//        Serial.println(WiFi.RSSI());  /*Print WiFi signal strength*/
//        rssi_micros_prev = current_micros;
//        reconnect_counter = 0;
//    }
}
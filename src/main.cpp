#include <Arduino.h>
#define LED_BUILTIN 2

#include "esp32-hal-cpu.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#endif

#include <Preferences.h>

Preferences preferences;

#include "SinricPro.h"
#include "SinricProBlinds.h"

#include "secrets.cpp"

SinricProBlinds &rightBlinds = SinricPro[RIGHT_BLINDS_ID];
SinricProBlinds &leftBlinds = SinricPro[LEFT_BLINDS_ID];

int blindsPosition = 0;
bool powerState = false;

//const int LED_BUILTIN = 2;
bool dir = true;
unsigned int step_position_right = 0;
unsigned int step_position_left = 0;
int delta_steps_right = 0;
int delta_steps_left = 0;
int delta_steps_prev_left = 0;
int delta_steps_prev_right = 0;
int dir_pin1 = 23;
int step_pin1 = 22;
int dir_pin2 = 18;
int step_pin2 = 19;
double steptime_high = 3000;
double steptime_low = 100;
double steptime_right = steptime_high;
double steptime_left = steptime_high;
const int powerOnPin = 17;
const int powerControlPin = 16;
const int top_steps = 17650;
bool lastblinds = 0;
unsigned int iterations_right = 0;
unsigned int iterations_left = 0;
double interpolation_length = 500;
double interpolation_factor = log10(steptime_low/steptime_high) / (interpolation_length * log10(double(1)/double(2)));
//double falling_factor = log10f(steptime_high/steptime_low) / (double(100) * log10f(double(3)/double(2)));


unsigned long current_micros = micros();
unsigned long step_micros_right = micros();
unsigned long step_micros_left = micros();
unsigned long previousmicrosWifi = micros();
unsigned long previousmicrosDot = micros();
unsigned long rssi_micros_prev = micros();
const unsigned long  reconnectInterval = 1000*1000*15;
unsigned int reconnect_counter = 0;

void Restart(){
    Serial.println("restarting in 5 Seconds");
    if(preferences.getUInt("steps_right") != step_position_right)
        preferences.putUInt("steps_right", step_position_right);
    if(preferences.getUInt("steps_left") != step_position_left)
        preferences.putUInt("steps_left", step_position_left);
    preferences.end();
    delay(5000);
    ESP.restart();
}

void Reconnect(){
    while (WiFi.status() != WL_CONNECTED) {
        current_micros = micros();

        if (current_micros - previousmicrosWifi >= reconnectInterval){
            if(reconnect_counter >= 10)
                Restart();

            Serial.println("");
            Serial.print("Reconnecting");
            WiFi.disconnect();
            WiFi.reconnect();
            reconnect_counter += 1;
            previousmicrosWifi = current_micros;
        }

        if (current_micros - previousmicrosDot >= 250000){
            Serial.printf(".");
            previousmicrosDot = current_micros;
        }
    }
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    Reconnect();
}

bool onPowerState(const String &deviceId, bool &state) {
    Serial.printf("Device %s power turned %s \r\n", deviceId.c_str(), state?"on":"off");
    powerState = state;
    return true; // request handled properly
}

bool onRangeValue(const String &deviceId, int &position) {
    Serial.println("Absolute");

    if(deviceId == "63dc458d22e49e3cb5f7617b"){
        delta_steps_right = map(position, 0, 100, 0, top_steps) - step_position_right;
        step_position_right += delta_steps_right;
        preferences.putUInt("steps_right", step_position_right);

    }else if(deviceId == "63dc45aa22e49e3cb5f761ab"){
        delta_steps_left += map(position, 0, 100, 0, top_steps) - step_position_left;
        step_position_left += delta_steps_left;
        preferences.putUInt("steps_left", step_position_left);
    }else
        return false;

    Serial.printf("Device %s set position to %d\r\n", deviceId.c_str(), position);
    return true; // request handled properly
}

bool onAdjustRangeValue(const String &deviceId, int &positionDelta) {

    Serial.println("Delta");

    if(deviceId == "63dc458d22e49e3cb5f7617b"){
        delta_steps_right = map(positionDelta, 0, 100, 0, top_steps);
        step_position_right += delta_steps_right;
        preferences.putUInt("steps_right", step_position_right);

    }else if(deviceId == "63dc45aa22e49e3cb5f761ab"){
        delta_steps_left = map(positionDelta, 0, 100, 0, top_steps);
        step_position_left += delta_steps_left;
        preferences.putUInt("steps_left", step_position_left);

    }else
        return false;

    blindsPosition += positionDelta;
    Serial.printf("Device %s position changed about %i to %d\r\n", deviceId.c_str(), positionDelta, blindsPosition);
    positionDelta = blindsPosition; // calculate and return absolute position
    return true; // request handled properly
}

// setup function for WiFi connection
void setupWiFi() {
    WiFi.disconnect(true);
    delay(1000);
    WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.printf("\r\n[Wifi]: Connecting");

    Reconnect();

    IPAddress localIP = WiFi.localIP();
    Serial.printf("connected!\r\n[WiFi]: IP-Address is %d.%d.%d.%d\r\n", localIP[0], localIP[1], localIP[2], localIP[3]);
}

void setupSinricPro() {
    // get a new Blinds device from SinricPro
    rightBlinds.onPowerState(onPowerState);
    rightBlinds.onRangeValue(onRangeValue);
    rightBlinds.onAdjustRangeValue(onAdjustRangeValue);

    // get a new Blinds device from SinricPro
    leftBlinds.onPowerState(onPowerState);
    leftBlinds.onRangeValue(onRangeValue);
    leftBlinds.onAdjustRangeValue(onAdjustRangeValue);

    // setup SinricPro
    SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); });
    SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
    SinricPro.begin(APP_KEY, APP_SECRET);
}

void setupBlinds(){
    step_position_right = preferences.getUInt("steps_right", 0);
    step_position_left = preferences.getUInt("steps_left", 0);
    pinMode(dir_pin1, OUTPUT);
    pinMode(step_pin1, OUTPUT);
    pinMode(dir_pin2, OUTPUT);
    pinMode(step_pin2, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(powerOnPin, OUTPUT);
    pinMode(powerControlPin, INPUT_PULLUP);

    digitalWrite(powerOnPin, 1);
    digitalWrite(step_pin1, 0);
    digitalWrite(step_pin2, 0);
    digitalWrite(LED_BUILTIN, false);
}

void powerOn(){
    digitalWrite(powerOnPin, false);
    while (!digitalRead(powerControlPin)){
        digitalWrite(LED_BUILTIN, false);}
}

void step(){
    int *step_pin;
    int *dir_pin;
    int *delta_steps;
    int *delta_steps_prev;
    double *steptime;
    unsigned int *iterations;
    bool blinds_running = false;

    if(delta_steps_right != 0 && (!lastblinds || delta_steps_left == 0)){
        steptime = &steptime_right;
        if(current_micros - step_micros_right >= *steptime){
            step_pin = &step_pin1;
            dir_pin = &dir_pin1;
            delta_steps = &delta_steps_right;
            delta_steps_prev = &delta_steps_prev_right;
            iterations = &iterations_right;
            step_micros_right = current_micros;
            //Serial.print("Right:");
            lastblinds = true;
            blinds_running = true;
        }
    }else if(delta_steps_left != 0 && (lastblinds || delta_steps_right == 0)){
        steptime = &steptime_left;
        if(current_micros - step_micros_left >= *steptime){
            //Serial.println((double)(current_micros - step_micros_left));
            step_pin = &step_pin2;
            dir_pin = &dir_pin2;
            delta_steps = &delta_steps_left;
            delta_steps_prev = &delta_steps_prev_left;
            iterations = &iterations_left;
            step_micros_left = current_micros;
            //Serial.print("Left:");
            lastblinds = false;
            blinds_running = true;
        }
    }else{
        iterations_right = 0;
        iterations_left = 0;
        digitalWrite(powerOnPin, true);
        return;
    }

    if(blinds_running){
        if(abs(*delta_steps) + *iterations < interpolation_length*2)
            *steptime = steptime_high;

        else if(*iterations > interpolation_length && abs(*delta_steps) > interpolation_length)
            *steptime = steptime_low;

        else if(*iterations <= interpolation_length)
            *steptime = steptime_high * pow((double(1)/double(2)), double(*iterations) * interpolation_factor);

        else if(abs(*delta_steps) <= interpolation_length)
            *steptime = steptime_high * pow((double(1)/double(2)), double(abs(*delta_steps)) * interpolation_factor);

        //else
        //  *steptime = steptime_low;

        //Serial.print(*iterations);
        //Serial.print(" | ");
        //Serial.println(*steptime);

        powerOn();

        if(*delta_steps > 0){
            digitalWrite(*dir_pin, 1);
            *delta_steps -= 1;
        }else{
            digitalWrite(*dir_pin, 0);
            *delta_steps += 1;
        }

        digitalWrite(*step_pin, 1);
        digitalWrite(*step_pin, 0);

        *iterations += 1;
    }
}

// main setup function
void setup() {
    setCpuFrequencyMhz(240); //Set CPU clock to 80MHz fo example

    Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
    preferences.begin("rollos", false);
    setupWiFi();
    setupSinricPro();
    setupBlinds();

    delay(500);
}

void loop() {
    SinricPro.handle();

    current_micros = micros();
    step();


    if(WiFi.status() == WL_CONNECTED && current_micros - rssi_micros_prev >= 30000000){
        Serial.print("Connected Network Signal Strength (RSSI): ");
        Serial.println(WiFi.RSSI());  /*Print WiFi signal strength*/
        rssi_micros_prev = current_micros;
        reconnect_counter = 0;
    }
}
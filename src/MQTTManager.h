//
// Created by maxim on 07.01.2026.
//

#ifndef DIY_SMART_HOME_BLINDS_MQTTMANAGER_H
#define DIY_SMART_HOME_BLINDS_MQTTMANAGER_H

#include <Arduino.h>
#include <AsyncMqttClient.h>

// Forward declarations
class Blind;

class MQTTManager {
public:
    MQTTManager(const IPAddress &mqtt_broker_ip, const String &mqtt_user = "", const String &mqtt_password = "");
    void handle();

private:
    static void onMqttConnect(bool sessionPresent);
    static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    static void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

    // Message handlers for different command types
    static void handleSetPositionCommand(Blind* blind, uint8_t targetPosition);
    static void handleOpenCommand(Blind* blind);
    static void handleCloseCommand(Blind* blind);
    static void handleStopCommand(Blind* blind);

    // Helper functions
    static Blind* findBlindById(int blindId);
    static int extractBlindIdFromTopic(const String& topic);

    static void publishDiscovery(Blind* blind);
    static void publishState(Blind* blind);
    static void publishPosition(Blind* blind, int8_t targetPosition = -1);  // -1 = use current position

    static AsyncMqttClient mqttClient;
    static unsigned long lastPositionUpdate;
};


#endif //DIY_SMART_HOME_BLINDS_MQTTMANAGER_H
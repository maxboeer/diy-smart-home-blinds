//
// Created by maxim on 07.01.2026.
//

#include "MQTTManager.h"

#include <map>
#include <WiFi.h>

#include "BlindManager.h"
#include "Blind.h"
#include "EEPROM.h"

// Static member initialization
AsyncMqttClient MQTTManager::mqttClient;
bool MQTTManager::isConnected = false;

MQTTManager::MQTTManager(const IPAddress &mqtt_broker_ip, const String &mqtt_user, const String &mqtt_password) {
    // Setup MQTT callbacks
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);

    // Configure MQTT broker
    mqttClient.setServer(mqtt_broker_ip, 1883);
    if (mqtt_user.length() > 0) {
        mqttClient.setCredentials(mqtt_user.c_str(), mqtt_password.c_str());
    }

    // Connect to MQTT broker
    Serial.println("[MQTT]: Connecting to broker...");
    mqttClient.connect();
}

void MQTTManager::handle() {
    // Handle reconnection if disconnected
    handleReconnect();

    // Check every 2 seconds for changes in target positions and publish state if changed
    static unsigned long lastCheck = 0;
    static std::map<int, size_t> lastTargetSizes;

    if (millis() - lastCheck > 2000) {
        for (auto blind : BlindManager::blinds) {
            size_t currentSize = blind->target_positions.size();

            // Check if target_positions changed
            if (lastTargetSizes.find(blind->id) == lastTargetSizes.end() ||
                lastTargetSizes[blind->id] != currentSize) {
                publishState(blind);
                lastTargetSizes[blind->id] = currentSize;
                }
        }
        lastCheck = millis();
    }

    // Publish position updates every 500ms, but only for blinds that are moving
    // if (millis() - lastPositionUpdate > 500) {
    //     for (auto blind : BlindManager::blinds) {
    //         // Only publish if blind is moving (has target positions)
    //         if (!blind->target_positions.empty()) {
    //             publishPosition(blind);
    //             publishState(blind);
    //         }
    //     }
    //     lastPositionUpdate = millis();
    // }
}

void MQTTManager::onMqttConnect(bool sessionPresent) {
    Serial.println("[MQTT]: Connected to broker");
    isConnected = true;
    digitalWrite(LED_BUILTIN, LOW);  // Turn off LED when connected

    // Publish Home Assistant Discovery for all blinds
    for (auto blind : BlindManager::blinds) {
        publishDiscovery(blind);

        // Subscribe to command topics
        String setTopic = "home/blinds/" + String(blind->id) + "/set";
        String setPositionTopic = "home/blinds/" + String(blind->id) + "/set_position";

        mqttClient.subscribe(setTopic.c_str(), 1);
        mqttClient.subscribe(setPositionTopic.c_str(), 1);

        Serial.printf("[MQTT]: Subscribed to %s\r\n", setTopic.c_str());
        Serial.printf("[MQTT]: Subscribed to %s\r\n", setPositionTopic.c_str());

        // Publish current state and position so they are not unknown in Home Assistant
        publishState(blind);
        publishPosition(blind);
        Serial.printf("[MQTT]: Published current state and position for Blind %d\r\n", blind->id);
    }
}

void MQTTManager::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.println("[MQTT]: Disconnected from broker");
    isConnected = false;
    // Reconnection is handled in handle() function
}

void MQTTManager::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    String topicStr = String(topic);
    String payloadStr;

    // Build payload string
    for (size_t i = 0; i < len; i++) {
        payloadStr += (char)payload[i];
    }

    Serial.printf("[MQTT]: Message received on %s: %s\r\n", topic, payloadStr.c_str());

    // Extract blind ID from topic
    int blindId = extractBlindIdFromTopic(topicStr);
    if (blindId == -1) {
        Serial.println("[MQTT]: Could not extract blind ID from topic");
        return;
    }

    // Find the blind
    Blind* targetBlind = findBlindById(blindId);
    if (!targetBlind) {
        Serial.printf("[MQTT]: Blind with ID %d not found\r\n", blindId);
        return;
    }

    // Route to appropriate handler based on topic
    if (topicStr.endsWith("/set_position")) {
        uint8_t targetPosition = payloadStr.toInt();
        handleSetPositionCommand(targetBlind, targetPosition);
    }
    else if (topicStr.endsWith("/set")) {
        if (payloadStr == "OPEN") {
            handleOpenCommand(targetBlind);
        }
        else if (payloadStr == "CLOSE") {
            handleCloseCommand(targetBlind);
        }
        else if (payloadStr == "STOP") {
            handleStopCommand(targetBlind);
        }
    }
}

// Helper function to extract blind ID from topic
int MQTTManager::extractBlindIdFromTopic(const String& topic) {
    // Topics: "home/blinds/X/set" or "home/blinds/X/set_position"
    int idStart = topic.indexOf("/blinds/") + 8;
    int idEnd = topic.indexOf("/", idStart);

    if (idStart > 7 && idEnd > idStart) {
        return topic.substring(idStart, idEnd).toInt();
    }

    return -1;
}

// Helper function to find blind by ID
Blind* MQTTManager::findBlindById(int blindId) {
    for (auto blind : BlindManager::blinds) {
        if (blind->id == blindId) {
            return blind;
        }
    }
    return nullptr;
}

// Handler for set position command
void MQTTManager::handleSetPositionCommand(Blind* blind, uint8_t targetPosition) {
    // Map position (0-100) to steps
    uint32_t targetSteps = map(targetPosition, 0, 100, blind->bottom_steps, blind->top_steps);
    blind->target_positions.push(targetSteps);
    EEPROM::storeUInt("steps_" + String(blind->id), targetSteps);

    Serial.printf("[MQTT]: Blind %d moving to position %d%% (%d steps)\r\n", blind->id, targetPosition, targetSteps);

    // Publish target position and state immediately (before motor starts)
    publishPosition(blind, targetPosition);
    publishState(blind);
}

// Handler for open command
void MQTTManager::handleOpenCommand(Blind* blind) {
    blind->target_positions.push(blind->top_steps);
    EEPROM::storeUInt("steps_" + String(blind->id), blind->top_steps);

    Serial.printf("[MQTT]: Blind %d opening\r\n", blind->id);

    // Publish target position and state immediately (before motor starts)
    publishPosition(blind, 100);
    publishState(blind);
}

// Handler for close command
void MQTTManager::handleCloseCommand(Blind* blind) {
    blind->target_positions.push(blind->bottom_steps);
    EEPROM::storeUInt("steps_" + String(blind->id), blind->bottom_steps);

    Serial.printf("[MQTT]: Blind %d closing\r\n", blind->id);

    // Publish target position and state immediately (before motor starts)
    publishPosition(blind, 0);
    publishState(blind);
}

// TODO: einfach alle rauspoppen könnte probleme geben (sollte in der Blind klasse überprüft werden)
// Handler for stop command
void MQTTManager::handleStopCommand(Blind* blind) {
    while (!blind->target_positions.empty()) {
        blind->target_positions.pop();
    }

    Serial.printf("[MQTT]: Blind %d stopped\r\n", blind->id);

    // Publish current position immediately
    publishPosition(blind);

    // Publish state immediately
    publishState(blind);
}

void MQTTManager::publishDiscovery(Blind* blind) {
    String discoveryTopic = "homeassistant/cover/blind_" + String(blind->id) + "/config";

    String payload = "{"
        "\"name\":\"Blind " + String(blind->id) + "\","
        "\"unique_id\":\"blind_" + String(blind->id) + "\","
        "\"device_class\":\"blind\","
        "\"command_topic\":\"home/blinds/" + String(blind->id) + "/set\","
        "\"state_topic\":\"home/blinds/" + String(blind->id) + "/state\","
        "\"position_topic\":\"home/blinds/" + String(blind->id) + "/position\","
        "\"set_position_topic\":\"home/blinds/" + String(blind->id) + "/set_position\","
        "\"position_open\":100,"
        "\"position_closed\":0,"
        "\"payload_open\":\"OPEN\","
        "\"payload_close\":\"CLOSE\","
        "\"payload_stop\":\"STOP\","
        "\"optimistic\":false"
    "}";

    mqttClient.publish(discoveryTopic.c_str(), 1, true, payload.c_str());
    Serial.printf("[MQTT]: Published discovery for Blind %d\r\n", blind->id);
}

void MQTTManager::publishState(Blind* blind) {
    String stateTopic = "home/blinds/" + String(blind->id) + "/state";

    String state;
    if (blind->target_positions.empty()) {
        state = "stopped";
    } else if (blind->target_positions.back() > blind->position) {
        state = "opening";
    } else if (blind->target_positions.back() < blind->position) {
        state = "closing";
    } else {
        state = "stopped";
    }

    mqttClient.publish(stateTopic.c_str(), 1, false, state.c_str());
}

void MQTTManager::publishPosition(Blind* blind, int8_t targetPosition) {
    String positionTopic = "home/blinds/" + String(blind->id) + "/position";

    uint8_t positionToPublish;

    // If targetPosition is specified, use it; otherwise use current position
    if (targetPosition >= 0) {
        positionToPublish = targetPosition;
    } else {
        // Map current position to 0-100 range
        positionToPublish = map(blind->position, blind->bottom_steps, blind->top_steps, 0, 100);
    }

    mqttClient.publish(positionTopic.c_str(), 1, false, String(positionToPublish).c_str());
}

void MQTTManager::handleReconnect() {
    static unsigned long lastLedBlink = 0;
    static bool ledState = LOW;
    static unsigned long lastReconnectAttempt = 0;

    // Check if we're connected
    if (isConnected) {
        return;  // Already connected, nothing to do
    }

    // WiFi must be connected first
    if (WiFi.status() != WL_CONNECTED) {
        return;  // WiFi not connected, wait for WiFi first
    }

    // Blink LED slowly (1 second interval) to indicate MQTT reconnecting
    unsigned long currentMillis = millis();
    if (currentMillis - lastLedBlink >= 1000) {
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState);
        lastLedBlink = currentMillis;
    }

    // Try to reconnect every 5 seconds
    if (currentMillis - lastReconnectAttempt >= 5000) {
        Serial.println("[MQTT]: Attempting to reconnect...");
        mqttClient.connect();
        lastReconnectAttempt = currentMillis;
    }
}


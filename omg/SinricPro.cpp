//
// Created by maxim on 05.12.2024.
//
#include "SinricPro.h"

using namespace SINRICPRO_NAMESPACE;

void SinricProClass::begin(String appKey, String appSecret, String serverURL) {
    bool success = true;
    if (!appKey.length()) {
        DEBUG_SINRIC("[SinricPro:begin()]: App-Key \"%s\" is invalid!! Please check your app-key!! SinricPro will not work!\r\n", appKey.c_str());
        success = false;
    }
    if (!appSecret.length()) {
        DEBUG_SINRIC("[SinricPro:begin()]: App-Secret \"%s\" is invalid!! Please check your app-secret!! SinricPro will not work!\r\n", appSecret.c_str());
        success = false;
    }

    if (!success) {
        _begin = false;
        return;
    }

    this->appKey    = appKey;
    this->appSecret = appSecret;
    this->serverURL = serverURL;
    _begin          = true;
    _udpListener.begin(&receiveQueue);
}

void SinricProClass::handle() {
    static bool begin_error = false;
    if (!_begin) {
        if (!begin_error) {  // print this only once!
            DEBUG_SINRIC("[SinricPro:handle()]: ERROR! SinricPro.begin() failed or was not called prior to event handler\r\n");
            DEBUG_SINRIC("[SinricPro:handle()]:    -Reasons include an invalid app-key, invalid app-secret or no valid deviceIds)\r\n");
            DEBUG_SINRIC("[SinricPro:handle()]:    -SinricPro is disabled! Check earlier log messages for details.\r\n");
            begin_error = true;
        }
        return;
}

void SinricProClass::stop() {
    _begin = false;
    DEBUG_SINRIC("[SinricPro:stop()\r\n");
    _websocketListener.stop();
}

bool SinricProClass::isConnected() {
    return _websocketListener.isConnected();
}

void SinricProClass::onConnected(ConnectedCallbackHandler cb) {
    _websocketListener.onConnected(cb);
}

void SinricProClass::onDisconnected(DisconnectedCallbackHandler cb) {
    _websocketListener.onDisconnected(cb);
}

void SinricProClass::onPong(PongCallback cb) {
    _websocketListener.onPong(cb);
}

void SinricProClass::restoreDeviceStates(bool flag) {
    _websocketListener.setRestoreDeviceStates(flag);
}

void SinricProClass::setResponseMessage(String&& message) {
    responseMessageStr = message;
}

unsigned long SinricProClass::getTimestamp() {
    return timestamp.getTimestamp();
}

SinricProClass::Proxy SinricProClass::operator[](const String deviceId) {
    return Proxy(this, deviceId);
}

void SinricProClass::onOTAUpdate(OTAUpdateCallbackHandler cb) {
    _moduleCommandHandler.onOTAUpdate(cb);
}

void SinricProClass::onSetSetting(SetSettingCallbackHandler cb) {
    _moduleCommandHandler.onSetSetting(cb);
}

void SinricProClass::onReportHealth(ReportHealthCallbackHandler cb) {
    _moduleCommandHandler.onReportHealth(cb);
}

template <typename DeviceType>
DeviceType& SinricProClass::add(String deviceId) {
    DeviceType* newDevice = new DeviceType(deviceId);
    DEBUG_SINRIC("[SinricPro:add()]: Adding device with id \"%s\".\r\n", deviceId.c_str());
    newDevice->begin(this);

    devices.push_back(newDevice);
    return *newDevice;
}

void SinricProClass::add(SinricProDeviceInterface& newDevice) {
    // Function definition
}

void SinricProClass::add(SinricProDeviceInterface* newDevice) {
    // Function definition
}

JsonDocument SinricProClass::prepareResponse(JsonDocument& requestMessage) {
    // Function definition
}

JsonDocument SinricProClass::prepareEvent(String deviceId, const char* action, const char* cause) {
    // Function definition
}

void SinricProClass::sendMessage(JsonDocument& jsonMessage) {
    // Function definition
}

void SinricProClass::handleReceiveQueue() {
    // Function definition
}

void SinricProClass::handleSendQueue() {
    // Function definition
}

void SinricProClass::handleDeviceRequest(JsonDocument& requestMessage, interface_t Interface) {
    // Function definition
}

void SinricProClass::handleModuleRequest(JsonDocument& requestMessage, interface_t Interface) {
    // Function definition
}

void SinricProClass::handleResponse(JsonDocument& responseMessage) {
    // Function definition
}

JsonDocument SinricProClass::prepareRequest(String deviceId, const char* action) {
    // Function definition
}

void SinricProClass::connect() {
    // Function definition
}

void SinricProClass::disconnect() {
    // Function definition
}

void SinricProClass::reconnect() {
    // Function definition
}

void SinricProClass::onConnect() {
    // Function definition
}

void SinricProClass::onDisconnect() {
    // Function definition
}

void SinricProClass::extractTimestamp(JsonDocument& message) {
    // Function definition
}

SinricProDeviceInterface* SinricProClass::getDevice(String deviceId) {
    // Function definition
}

template <typename DeviceType>
DeviceType& SinricProClass::getDeviceInstance(String deviceId) {
    // Function definition
}

SinricProClass::Proxy::Proxy(SinricProClass* ptr, const String& deviceId)
        : ptr(ptr), deviceId(deviceId) {}

template <typename DeviceType>
SinricProClass::Proxy::operator DeviceType&() {
    // Function definition
}

SinricProClass SinricPro;
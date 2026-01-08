//
// Created by maxim on 05.12.2024.
//
#include "secrets.h"

struct secretsNamespace::_secrets secretsExample = {
        "EXAMPLE_WIFI_SSID",
        "EXAMPLE_WIFI_PASSWORD",

        IPAddress(192, 168, 178, 100),
        "EXAMPLE_MQTT_USER",
        "EXAMPLE_MQTT_PASSWORD",

        OTA_HOSTNAME,
        3232,
        OTA_PASSWORD,
};
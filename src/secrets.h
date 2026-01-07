//
// Created by maxim on 05.12.2024.
//

#ifndef DIY_SMART_HOME_BLINDS_SECRETS_H
#define DIY_SMART_HOME_BLINDS_SECRETS_H

#include <Arduino.h>

namespace secretsNamespace {
    struct _wifi {
        const String ssid;
        const String pass;
    };

    struct _sinric {
        const String app_key;
        const String app_secret;
        const String right_blinds_id;
        const String left_blinds_id;
        const int baud_rate;
    };

    struct _ota {
        const String hostname;
        const uint16_t port;
        const String password;
    };

    struct _secrets {
        struct _wifi wifi;
        struct _sinric sinric;
        struct _ota ota;

        _secrets(const String& wifi_ssid, const String& wifi_pass, const String& sinric_app_key, const String& sinric_app_secret, const String& right_blinds_id, const String& left_blinds_id, const int& sinric_baud_rate, const String& ota_hostname, const uint16_t& ota_port, const String& ota_password)
            : wifi{wifi_ssid, wifi_pass}, sinric{sinric_app_key, sinric_app_secret, right_blinds_id, left_blinds_id, sinric_baud_rate}, ota{ota_hostname, ota_port, ota_password} {}
    };
}

extern secretsNamespace::_secrets secrets;

#endif //DIY_SMART_HOME_BLINDS_SECRETS_H

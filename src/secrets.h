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

    struct _secrets {
        struct _wifi wifi;
        struct _sinric sinric;

        _secrets(const String& wifi_ssid, const String& wifi_pass, const String& sinric_app_key, const String& sinric_app_secret, const String& right_blinds_id, const String& left_blinds_id, int sinric_baud_rate)
            : wifi{wifi_ssid, wifi_pass}, sinric{sinric_app_key, sinric_app_secret, right_blinds_id, left_blinds_id, sinric_baud_rate} {}
    };
}

extern secretsNamespace::_secrets secrets;

#endif //DIY_SMART_HOME_BLINDS_SECRETS_H

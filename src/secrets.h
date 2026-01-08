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

    struct _mqtt {
        const IPAddress broker_ip;
        const String user;
        const String password;
    };

    struct _ota {
        const String hostname;
        const uint16_t port;
        const String password;
    };

    struct _secrets {
        struct _wifi wifi;
        struct _mqtt mqtt;
        struct _ota ota;

        _secrets(const String& wifi_ssid, const String& wifi_pass, const IPAddress& mqtt_broker_ip, const String& mqtt_user, const String& mqtt_pass, const String& ota_hostname, const uint16_t& ota_port, const String& ota_password)
            : wifi{wifi_ssid, wifi_pass}, mqtt{mqtt_broker_ip, mqtt_user, mqtt_pass}, ota{ota_hostname, ota_port, ota_password} {}
    };
}

extern secretsNamespace::_secrets secrets;

#endif //DIY_SMART_HOME_BLINDS_SECRETS_H

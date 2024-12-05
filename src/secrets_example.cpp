//
// Created by maxim on 05.12.2024.
//
#include <string>

namespace secretsNamespace {
    struct _wifi {
        const char* ssid;
        const char* pass;
    };

    struct _sinric {
        const char* app_key;
        const char* app_secret;
        const char* right_blinds_id;
        const char* left_blinds_id;
        const int baud_rate;
    };

    struct _secrets {
        struct _wifi wifi;
        struct _sinric sinric;
    };
}

constexpr struct secretsNamespace::_secrets secrets = {
    .wifi = {
        .ssid = "YOURSSID",
        .pass = "YOURPASS"
    },
    .sinric = {
        .app_key = "YOURAPPKEY",
        .app_secret = "YOURAPPSECRET",
        .right_blinds_id = "YOURRIGHTBLINDSID",
        .left_blinds_id = "YOURLEFTBLINDSID",
        .baud_rate = 500000
    }
};
#pragma once
#include <PubSubClient.h>

// #define USE_WIFI_MANAGER

namespace client {
    void init(void);
    bool topic_matches_sub(const char *topic, const char *sub);   
    bool reconnect(void);
    bool publish(const char *topic, const byte *message, unsigned int len);
    bool loop(void);

    // Network info
    static const char* ssid = "laptop";
    static const char* pass = "cowsarereallycool";

    // Broker info
    static const char* mqtt_broker_ip = "10.63.212.28";
    static const int mqtt_broker_port = 1883;
}

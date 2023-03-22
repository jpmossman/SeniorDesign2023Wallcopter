cd ~/#include "client.hpp"
#include <WiFi.h>
#include <vector>
#include <string>
#include <WiFiManager.h>
// ~~~~~~~~~~~~~~~~~~~~~~~ Private Variable Declarations ~~~~~~~~~~~~~~~~~~~~~~

// Broker info
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// MQTT Client info
const char* clientID = "Wallcopter";
const char* willTopic = "wc/status";
const uint8_t willQos = 1;
const boolean willRetain = true;
const char* willMessage = "offline";

// ~~~~~~~~~~~~~~~~~~~~~~~ Private Function Declarations ~~~~~~~~~~~~~~~~~~~~~~

void on_message(const char *topic, const byte *message, unsigned int len);

// ~~~~~~~~~~~~~~~~~~~~~~~ Public Function Definitions ~~~~~~~~~~~~~~~~~~~~~~~~

void client::init(void) {
    // Connect WiFi
    delay(10);
    #ifdef USE_WIFI_MANAGER
        // Use WiFiManager to login to WiFi
        WiFi.mode(WIFI_STA);
        WiFiManager wm;
        // wm.resetSettings();
        bool res = wm.autoConnect("Wallcopter AP");
        if (!res) {
            Serial.println("Failed to connect to WiFi");
        }
        else {
            Serial.println("Connected to WiFi");
        }
    #else
        // Connect to a WiFi network
        Serial.println("");
        Serial.print("Connecting to ");
        Serial.print(ssid);
        WiFi.begin(ssid, pass);

        // Wait for a successful connection...
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }

        // Announce a successful connection
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    #endif

    // Start MQTT client
    mqtt_client.setServer(mqtt_broker_ip, mqtt_broker_port);
    mqtt_client.setCallback(on_message);

    // Connect client
    while (!mqtt_client.connected()) {
        reconnect();
    }

    // Default max size is 256, much too small for images (to be fair, we REALLY shouldnt be using MQTT for images)
    mqtt_client.setBufferSize(8192);
}

bool client::topic_matches_sub(const char *topic, const char *sub) {
    return true;
}

bool client::reconnect(void) {
    if (!mqtt_client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqtt_client.connect(clientID, willTopic, willQos, willRetain, willMessage)) {
            Serial.println("connected");
            mqtt_client.publish(willTopic,"online",true);
        }
        else {
            Serial.print("failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
    return false;
}

bool client::publish(const char *topic, const byte *message, unsigned int len) {
    return mqtt_client.publish(topic, message, len);
}

bool client::loop(void) {
    while (!mqtt_client.connected()) {
        reconnect();
    }
    mqtt_client.loop();
    return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~ Private Function Definitions ~~~~~~~~~~~~~~~~~~~~~~~

void on_message(const char *topic, const byte *message, unsigned int len){
    // Print info about message and parse into String
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;
    for (size_t i = 0; i < len; i++) {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println("");
}

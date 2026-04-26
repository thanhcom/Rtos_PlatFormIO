#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include "DataModels.h"
#include "ConfigManager.h"

class MqttModule {
private:
    WiFiClient espClient;
    PubSubClient client;
    ConfigManager *_config;

    // biến runtime (không hardcode nữa)
    String mqtt_server;
    int    mqtt_port;
    String mqtt_user;
    String mqtt_pass;
    String clientID;

    const char* topic_pub = "testTopic";
    const char* topic_sub = "testTopic/control";

public:
    MqttModule() : client(espClient) {}

    // 👇 thêm config vào đây
    void setup(ConfigManager &config) {
        _config = &config;

        // đọc từ Preferences
        mqtt_server = _config->getString("mqtt_server", "192.168.1.66");
        mqtt_port   = _config->getString("mqtt_port", "1882").toInt();
        mqtt_user   = _config->getString("mqtt_user", "");
        mqtt_pass   = _config->getString("mqtt_pass", "");
        clientID    = _config->getString("client_id", "ESP32");

        client.setServer(mqtt_server.c_str(), mqtt_port);
        client.setBufferSize(512);
        client.setCallback(callback);

        Serial.println("[MQTT] Config loaded");
    }

    static void callback(char* topic, byte* payload, unsigned int length) {
        Serial.print("\n[MQTT] Topic: ");
        Serial.println(topic);

        String message;
        for (int i = 0; i < length; i++) message += (char)payload[i];

        Serial.println("[MQTT] Msg: " + message);

        if (message == "ON") digitalWrite(2, HIGH);
        else if (message == "OFF") digitalWrite(2, LOW);
    }

    bool connect() {
        if (!client.connected()) {
            Serial.printf("[MQTT] Connecting to %s:%d...\n", mqtt_server.c_str(), mqtt_port);

            if (client.connect(
                    clientID.c_str(),
                    mqtt_user.c_str(),
                    mqtt_pass.c_str()
                )) {

                Serial.println("[MQTT] Connected!");
                client.subscribe(topic_sub);

            } else {
                Serial.printf("[MQTT] Failed, rc=%d\n", client.state());
            }
        }
        return client.connected();
    }

    void publishData(float temp, float hum) {
        if (client.connected()) {
            char msg[64];
            snprintf(msg, sizeof(msg),
                     "{\"temp\":%.1f,\"hum\":%.1f}", temp, hum);

            if (client.publish(topic_pub, msg)) {
                Serial.printf("[MQTT] Pub [%s]: %s\n", topic_pub, msg);
            } else {
                Serial.println("[MQTT] Publish fail!");
            }
        }
    }

    // Hàm mới: Chỉ gửi dữ liệu PZEM, không đụng đến hàm cũ
    void publishPzemData(const PzemData &pzem) {
        if (client.connected() && pzem.isValid) {
            char msg[256];
            // Đóng gói tất cả thông số điện năng
            snprintf(msg, sizeof(msg),
                     "{\"volt\":%.1f,\"amp\":%.2f,\"watt\":%.1f,\"energy\":%.2f,\"freq\":%.1f,\"pf\":%.2f}",
                     pzem.voltage, 
                     pzem.current, 
                     pzem.power, 
                     pzem.energy, 
                     pzem.frequency, 
                     pzem.pf);

            if (client.publish("pzem/data", msg)) { // Bạn có thể đổi topic tùy ý
                Serial.printf("[MQTT] Pub PZEM: %s\n", msg);
            } else {
                Serial.println("[MQTT] Pub PZEM fail!");
            }
        }
    }

    void loop() {
        client.loop();
    }
};
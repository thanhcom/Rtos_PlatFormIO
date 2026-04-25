#ifndef WIFI_MANAGER_MODULE_H
#define WIFI_MANAGER_MODULE_H

#include <WiFi.h>
#include <WiFiManager.h>
#include "ConfigManager.h"

class WifiManagerModule {
private:
    ConfigManager *_config;

    // buffer tạm cho WiFiManager
    char mqtt_server[40];
    char mqtt_port[6];
    char mqtt_user[32];
    char mqtt_pass[32];
    char client_id[32];

public:
    void begin(ConfigManager &config, const char* apName = "ESP32-Setup", const char* apPass = "12345678") {
        _config = &config;

        WiFiManager wm;

        // ===== Load từ Preferences =====
        strcpy(mqtt_server, _config->getString("mqtt_server", "192.168.1.66").c_str());
        strcpy(mqtt_port,   _config->getString("mqtt_port", "1882").c_str());
        strcpy(mqtt_user,   _config->getString("mqtt_user", "").c_str());
        strcpy(mqtt_pass,   _config->getString("mqtt_pass", "").c_str());
        strcpy(client_id,   _config->getString("client_id", "ESP32").c_str());

        // ===== Tạo field nhập =====
        WiFiManagerParameter p_server("server", "MQTT Server", mqtt_server, 40);
        WiFiManagerParameter p_port("port", "MQTT Port", mqtt_port, 6);
        WiFiManagerParameter p_user("user", "MQTT User", mqtt_user, 32);
        WiFiManagerParameter p_pass("pass", "MQTT Pass", mqtt_pass, 32);
        WiFiManagerParameter p_client("cid", "Client ID", client_id, 32);

        wm.addParameter(&p_server);
        wm.addParameter(&p_port);
        wm.addParameter(&p_user);
        wm.addParameter(&p_pass);
        wm.addParameter(&p_client);

        wm.setConnectRetries(3);        // thử lại 3 lần
        wm.setConnectTimeout(30);        // mỗi lần thử chờ 30 giây
        wm.setConfigPortalTimeout(180);  // chỉ bật AP nếu không kết nối sau 3 phút

        if (!wm.autoConnect(apName, apPass)) {
            Serial.println("[WiFi] Failed -> restart");
            ESP.restart();
        }

        Serial.println("[WiFi] Connected");

        // ===== Lưu lại sau khi user nhập =====
        _config->setString("mqtt_server", p_server.getValue());
        _config->setString("mqtt_port",   p_port.getValue());
        _config->setString("mqtt_user",   p_user.getValue());
        _config->setString("mqtt_pass",   p_pass.getValue());
        _config->setString("client_id",   p_client.getValue());

        Serial.println("[Config] MQTT saved");
    }

    bool isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }

    void loop() {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[WiFi] Lost -> reconnect");
            WiFi.reconnect();
        }
    }
};

#endif
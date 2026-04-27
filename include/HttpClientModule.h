#pragma once

#include <Arduino.h>
#include <esp_http_client.h>
#include <string>

class HttpClientModule {
public:
    typedef void (*HttpCallback)(const std::string &response, int statusCode);

    HttpClientModule() {}

    // --- CÁC HÀM PUBLIC ĐỂ MÀY GỌI TỪ MAIN ---

    void get(const char *url, HttpCallback callback = nullptr) {
        _request(HTTP_METHOD_GET, url, "", callback);
    }

    void post(const char *url, const std::string &payload, HttpCallback callback = nullptr) {
        _request(HTTP_METHOD_POST, url, payload, callback);
    }

    void put(const char *url, const std::string &payload, HttpCallback callback = nullptr) {
        _request(HTTP_METHOD_PUT, url, payload, callback);
    }

    void patch(const char *url, const std::string &payload, HttpCallback callback = nullptr) {
        _request(HTTP_METHOD_PATCH, url, payload, callback);
    }

    void del(const char *url, HttpCallback callback = nullptr) {
        _request(HTTP_METHOD_DELETE, url, "", callback);
    }

private:
    struct RequestParams {
        esp_http_client_method_t method;
        std::string url;
        std::string payload;
        HttpCallback callback;
    };

    struct ResponseBuffer {
        std::string data;
    };

    // Hàm private khởi tạo Task chung cho tất cả các method
    void _request(esp_http_client_method_t method, const char *url, const std::string &payload, HttpCallback callback) {
        RequestParams *params = new RequestParams();
        params->method = method;
        params->url = std::string(url);
        params->payload = payload;
        params->callback = callback;

        xTaskCreatePinnedToCore(_httpTask, "http_task", 8192, (void *)params, 2, NULL, 1);
    }

    static esp_err_t _httpEventHandler(esp_http_client_event_t *evt) {
        ResponseBuffer *resBuf = (ResponseBuffer *)evt->user_data;
        if (evt->event_id == HTTP_EVENT_ON_DATA && evt->data_len > 0) {
            resBuf->data.append((char *)evt->data, evt->data_len);
        }
        return ESP_OK;
    }

    static void _httpTask(void* pvParameters) {
        RequestParams* params = (RequestParams*)pvParameters;
        ResponseBuffer resBuf;

        esp_http_client_config_t config = {}; 
        config.url = params->url.c_str();
        config.event_handler = _httpEventHandler;
        config.user_data = &resBuf;
        config.timeout_ms = 10000;
        config.method = params->method;
        config.transport_type = HTTP_TRANSPORT_OVER_TCP; // Chỉ chơi HTTP cho lành

        esp_http_client_handle_t client = esp_http_client_init(&config);

        if (client != NULL) {
            esp_http_client_set_header(client, "User-Agent", "ESP32-Client");
            esp_http_client_set_header(client, "Content-Type", "application/json");

            // Nếu có dữ liệu gửi đi (POST/PUT/PATCH)
            if (!params->payload.empty()) {
                esp_http_client_set_post_field(client, params->payload.c_str(), params->payload.length());
            }

            esp_err_t err = esp_http_client_perform(client);
            int statusCode = (err == ESP_OK) ? esp_http_client_get_status_code(client) : -1;

            if (params->callback != nullptr) {
                params->callback(resBuf.data, statusCode);
            }

            esp_http_client_cleanup(client);
        }

        delete params; 
        vTaskDelete(NULL); 
    }
};
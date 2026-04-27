#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "WifiManagerModule.h"
#include "ConfigManager.h"
#include "MqttModule.h"
#include "DataModels.h"
#include "PzemModule.h"
//#include "LcdModule.h"
#include "HttpClientModule.h"

// Khai báo Queue Handle để chuyển SystemMessage giữa các Task
QueueHandle_t dataQueue;

// Khởi tạo các Module hệ thống
WifiManagerModule wifi;
MqttModule mqtt;
ConfigManager config;
PzemModule pzem(Serial1, 20, 21); // Sử dụng Serial1, chân RX:20, TX:21

//LiquidCrystal_I2C lcd(0x27, 16, 2); // Địa chỉ I2C: 0x27, số cột: 16, số hàng: 2
//LcdModule lcdModule(lcd);

HttpClientModule http;

// Hàm callback này sẽ được module gọi khi nhận xong data của HTTP Request
void myDataHandler(const std::string& response, int statusCode) {
    Serial.println("\n--- KẾT QUẢ HTTP ---");
    if (statusCode == 200) {
        Serial.printf("Dữ liệu: %s\n", response.c_str());
    } else {
        Serial.printf("Lỗi mẹ rồi! Code: %d\n", statusCode);
    }
}

// ================= TASK 1: ĐỌC PZEM (Core 1 - Ưu tiên 2) =================
void pzemTask(void *pv)
{
    pzem.begin();
    SystemMessage msg;
    msg.type = TYPE_PZEM;

    for (;;)
    {
        PzemData pzemRaw = pzem.readAll();
        if (pzemRaw.isValid)
        {
            msg.pzem = pzemRaw;
            // Đẩy dữ liệu vào hàng đợi, chờ tối đa 100ms nếu hàng đợi đầy
            xQueueSend(dataQueue, &msg, pdMS_TO_TICKS(100));
        }
        else
        {
            Serial.println("[PZEM] Read Error!");
        }
        // PZEM v3 phản hồi ~200ms, đọc mỗi 3s là tối ưu cho gia đình
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ================= TASK 2: ĐỌC CẢM BIẾN MÔI TRƯỜNG (Core 1 - Ưu tiên 1) =================
void sensorTask(void *pv)
{
    SystemMessage msg;
    msg.type = TYPE_ENV_SENSOR;

    for (;;)
    {
        // Giả lập hoặc đọc dữ liệu từ DHT/BME
        msg.env.temp = random(200, 350) / 10.0;
        msg.env.hum = random(400, 800) / 10.0;

        xQueueSend(dataQueue, &msg, pdMS_TO_TICKS(100));
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ================= TASK 3: MQTT & WIFI DISPATCHER (Core 0 - Ưu tiên 1) =================
void mqttTask(void *pv)
{
    SystemMessage rxMsg;
    char buf[17];

    Serial.println("[MQTT] Dispatcher Task started");

    for (;;)
    {
        if (!xQueueReceive(dataQueue, &rxMsg, portMAX_DELAY))
            continue;

        if (wifi.isConnected())
        {
            // giữ nguyên logic cũ: connect mỗi lần
            if (mqtt.connect())
            {
                mqtt.loop();

                switch (rxMsg.type)
                {
                case TYPE_PZEM:
                {
                    mqtt.publishPzemData(rxMsg.pzem);

                    //snprintf(buf, sizeof(buf), "P:%.1fW", rxMsg.pzem.power);
                    //lcdModule.printAt(buf, 0, 0);

                    Serial.printf("[MQTT] PZEM: %.1fV\n", rxMsg.pzem.voltage);
                    break;
                }

                case TYPE_ENV_SENSOR:
                {
                    mqtt.publishData(rxMsg.env.temp, rxMsg.env.hum);

                    //snprintf(buf, sizeof(buf), "T:%.1fC H:%.1f%%",
                    //         rxMsg.env.temp, rxMsg.env.hum);
                    //lcdModule.printAt(buf, 1, 0);

                    Serial.printf("[MQTT] Env: T:%.1f H:%.1f\n",
                                  rxMsg.env.temp, rxMsg.env.hum);
                    break;
                }

                case TYPE_RF_REMOTE:
                {
                    //snprintf(buf, sizeof(buf), "RF:%s", rxMsg.rf.code);
                    //lcdModule.printAt(buf, 0, 0);

                    Serial.printf("[MQTT] RF: %s\n", rxMsg.rf.code);
                    break;
                }

                default:
                    break;
                }
            }
        }
        else
        {
            Serial.println("[MQTT] WiFi Disconnected. Packet dropped.");
        }
    }
}

// ================= SETUP =================
void setup()
{
    Serial.begin(115200);
    pinMode(2, OUTPUT); // LED tích hợp để test điều khiển từ MQTT
    // Tạo Queue chứa được 15 gói tin SystemMessage
    // Sử dụng kích thước của struct đã tối ưu bằng Union
    dataQueue = xQueueCreate(15, sizeof(SystemMessage));

    if (dataQueue == NULL)
    {
        Serial.println("Failed to create Queue!");
        while (1)
            ;
    }

    // Khởi tạo các dịch vụ nền
    config.begin();
    wifi.begin(config);
    mqtt.setup(config);
    //lcdModule.begin();

    // Task xử lý phần cứng (PZEM và Sensor) chạy trên Core 1
    xTaskCreatePinnedToCore(pzemTask, "pzemTask", 3072, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(sensorTask, "sensorTask", 2048, NULL, 1, NULL, 1);

    // Task xử lý truyền thông (WiFi/MQTT) chạy trên Core 0
    xTaskCreatePinnedToCore(mqttTask, "mqttTask", 5120, NULL, 1, NULL, 0);

    // Test HTTP Client Module
    if(wifi.isConnected()) {
        http.get("http://thanhcom1989.ddns.net:1880/api/getbyid/2", myDataHandler);
    }   else {
        Serial.println("[HTTP] Cannot perform request, WiFi not connected.");
    }   
}

void loop()
{
    // Xóa Task loop mặc định để giải phóng tài nguyên cho các Task RTOS
    vTaskDelete(NULL);
}
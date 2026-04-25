#ifndef DATA_MODELS_H
#define DATA_MODELS_H

#include <Arduino.h>

// 1. Phân loại gói tin rõ ràng
enum DataType { 
    TYPE_PZEM, 
    TYPE_ENV_SENSOR, 
    TYPE_RF_REMOTE 
};

// 2. Dữ liệu PZEM
struct PzemData {
    float voltage;
    float current;
    float power;
    float energy;
    float frequency;
    float pf;
    bool isValid;

    PzemData() : voltage(0), current(0), power(0), energy(0), frequency(0), pf(0), isValid(false) {}
};

// 3. Dữ liệu cảm biến môi trường (Temp/Hum)
struct EnvData {
    float temp;
    float hum;
};

// 4. Dữ liệu RF
struct RFData {
    char code[64];
};

// 5. CHUẨN HÓA GÓI TIN QUEUE (Sử dụng Union để tối ưu bộ nhớ)
struct SystemMessage {
    DataType type; 
    union {
        PzemData pzem;
        EnvData  env;
        RFData   rf;
    };

    // "Tao sẽ tự lo việc khởi tạo, mày đừng tự xóa hàm này nữa"
    SystemMessage() {}
};

// 6. Trạng thái hệ thống (Để lưu trữ snapshot cuối cùng)
struct SystemState {
    PzemData lastPzem;
    EnvData  lastEnv;
};

#endif
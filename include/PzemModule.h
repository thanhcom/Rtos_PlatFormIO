#pragma once

#include <PZEM004Tv30.h>
#include <HardwareSerial.h>
#include "DataModels.h" // Chứa struct PzemData đã định nghĩa trước đó

class PzemModule {
private:
    PZEM004Tv30 _pzem;
    HardwareSerial* _pzemSerial;
    int _rx, _tx;

public:
    /**
     * @param serial: Sử dụng cổng HardwareSerial (ví dụ: Serial1)
     * @param rx: Chân RX (ESP32-C3 thường là 20)
     * @param tx: Chân TX (ESP32-C3 thường là 21)
     */
    PzemModule(HardwareSerial& serial, int rx, int tx) 
        : _pzem(serial, rx, tx), _pzemSerial(&serial), _rx(rx), _tx(tx) {}

    /**
     * Khởi tạo Serial cho PZEM
     */
    void begin() {
        // PZEM004Tv30 mặc định chạy 9600
        _pzemSerial->begin(9600, SERIAL_8N1, _rx, _tx);
    }

    /**
     * Đọc toàn bộ thông số và trả về struct PzemData
     * Hàm này được thiết kế để Task cảm biến gọi định kỳ
     */
    PzemData readAll() {
        PzemData data;

        // Đọc các giá trị từ cảm biến
        data.voltage   = _pzem.voltage();
        data.current   = _pzem.current();
        data.power     = _pzem.power();
        data.energy    = _pzem.energy();
        data.frequency = _pzem.frequency();
        data.pf        = _pzem.pf();

        // Kiểm tra tính hợp lệ: Nếu voltage là NAN nghĩa là chưa kết nối hoặc lỗi UART
        if (isnan(data.voltage)) {
            data.isValid = false;
        } else {
            data.isValid = true;
        }

        return data;
    }

    /**
     * Reset chỉ số điện năng tiêu thụ (Energy)
     * Có thể gọi từ MQTT Callback
     */
    bool resetEnergy() {
        return _pzem.resetEnergy();
    }

    /**
     * Thay đổi địa chỉ của PZEM (dùng khi mắc nhiều module trên cùng 1 đường bus)
     */
    bool setAddress(uint8_t addr) {
        return _pzem.setAddress(addr);
    }
};

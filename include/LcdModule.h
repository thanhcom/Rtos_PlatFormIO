#pragma once

#include <LiquidCrystal_I2C.h>

class LcdModule {
private:
    LiquidCrystal_I2C &_lcd;
    uint8_t _cols;
    uint8_t _rows;

    char _buffer[2][17]; // cho 16x2

public:
    // DI qua reference
    LcdModule(LiquidCrystal_I2C &lcd, uint8_t cols = 16, uint8_t rows = 2)
        : _lcd(lcd), _cols(cols), _rows(rows) {}

    void begin() {
        _lcd.init();
        _lcd.backlight();
        clear();

        printAt("System Starting", 0, 0);
    }

    void clear() {
        _lcd.clear();
        for (int r = 0; r < _rows; r++) {
            memset(_buffer[r], 0, sizeof(_buffer[r]));
        }
    }

    void print(const char* text) {
        printAt(text, 0, 0);
    }

    void printAt(const char* text, uint8_t row, uint8_t col) {
        if (row >= _rows || col >= _cols) return;

        for (uint8_t i = 0; i < _cols - col && text[i] != '\0'; i++) {
            if (_buffer[row][col + i] != text[i]) {
                _lcd.setCursor(col + i, row);
                _lcd.write(text[i]);
                _buffer[row][col + i] = text[i];
            }
        }
    }
};
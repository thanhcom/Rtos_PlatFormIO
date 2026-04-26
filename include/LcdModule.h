#ifndef LCD_MODULE_H
#define LCD_MODULE_H    
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  
class LcdModule {
private:
    LiquidCrystal_I2C lcd;

public:
    LcdModule(uint8_t addr = 0x27, uint8_t cols = 16, uint8_t rows = 2);
    void begin();
    void print(const String& text);
    void printAt(const String& text , int row , int col);
    void clear();
};

LcdModule::LcdModule(uint8_t addr, uint8_t cols, uint8_t rows) : lcd(addr, cols, rows) {}// Khởi tạo LCD với địa chỉ I2C và kích thước  

void LcdModule::begin() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Starting");
}
void LcdModule::print(const String& text) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(text);
}

void LcdModule::printAt(const String& text , int row , int col) {
    lcd.clear();
    lcd.setCursor(col, row);
    lcd.print(text);
}
void LcdModule::clear() {
    lcd.clear();
}       
#endif // LCD_MODULE_H  
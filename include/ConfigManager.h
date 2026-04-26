#pragma once

#include <Preferences.h>

class ConfigManager {
private:
    Preferences _prefs;

public:
    void begin() {
        _prefs.begin("app-config", false);
    }

    // ===== FLOAT =====
    void setTempThreshold(float threshold) {
        _prefs.putFloat("temp_limit", threshold);
    }

    float getTempThreshold() {
        return _prefs.getFloat("temp_limit", 30.0);
    }

    // ===== BOOL =====
    void setAutoMode(bool enable) {
        _prefs.putBool("auto_mode", enable);
    }

    bool getAutoMode() {
        return _prefs.getBool("auto_mode", true);
    }

    // ===== STRING (CÁI MÀY THIẾU) =====
    void setString(const char* key, const char* value) {
        _prefs.putString(key, value);
    }

    String getString(const char* key, const char* defaultValue = "") {
        return _prefs.getString(key, defaultValue);
    }

    void clear() {
    _prefs.clear();
    }
};
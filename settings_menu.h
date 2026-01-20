/*
 * Settings Menu Application
 * 
 * Controls:
 * - WiFi On/Off
 * - Bluetooth On/Off  
 * - Screen Brightness
 * - System Info
 * - Storage Info
 * 
 * Add this to your main project or save as settings_menu.h
 */

#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

#include <lvgl.h>
#include <WiFi.h>
#include <BluetoothSerial.h>
#include "SD_MMC.h"

// External reference to TFT for brightness control
extern TFT_eSPI my_lcd;

// Settings state
bool wifiEnabled = false;
bool bluetoothEnabled = false;
uint8_t screenBrightness = 128; // 0-255

// Bluetooth object
BluetoothSerial SerialBT;

// UI objects
lv_obj_t * wifi_switch = NULL;
lv_obj_t * bt_switch = NULL;
lv_obj_t * brightness_slider = NULL;
lv_obj_t * brightness_label = NULL;
lv_obj_t * wifi_status_label = NULL;
lv_obj_t * bt_status_label = NULL;
lv_obj_t * storage_label = NULL;

// ═══════════════════════════════════════════════════════════════
// SYSTEM CONTROL FUNCTIONS
// ═══════════════════════════════════════════════════════════════

void setWiFi(bool enable) {
    wifiEnabled = enable;
    
    if (enable) {
        WiFi.mode(WIFI_STA);
        Serial.println("WiFi enabled");
        lv_label_set_text(wifi_status_label, "WiFi: Initializing...");
    } else {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        Serial.println("WiFi disabled");
        lv_label_set_text(wifi_status_label, "WiFi: Off");
    }
}

void setBluetooth(bool enable) {
    bluetoothEnabled = enable;
    
    if (enable) {
        if (!SerialBT.begin("ESP32-S3 Device")) {
            Serial.println("Bluetooth init failed");
            lv_label_set_text(bt_status_label, "BT: Error");
            bluetoothEnabled = false;
        } else {
            Serial.println("Bluetooth enabled");
            lv_label_set_text(bt_status_label, "BT: ESP32-S3 Device");
        }
    } else {
        SerialBT.end();
        Serial.println("Bluetooth disabled");
        lv_label_set_text(bt_status_label, "BT: Off");
    }
}

void setScreenBrightness(uint8_t brightness) {
    screenBrightness = brightness;
    
    // TFT_eSPI doesn't have direct brightness control
    // You can implement PWM on backlight pin if available
    // For now, we'll store the value for other uses
    
    Serial.printf("Brightness set to: %d%%\n", (brightness * 100) / 255);
    
    static char text[32];
    snprintf(text, sizeof(text), "Brightness: %d%%", (brightness * 100) / 255);
    lv_label_set_text(brightness_label, text);
}

// ═══════════════════════════════════════════════════════════════
// SETTINGS MENU UI
// ═══════════════════════════════════════════════════════════════

void updateStorageInfo() {
    if (!storage_label) return;
    
    static char storage_text[128];
    
    // ESP32 Flash info
    uint32_t flashSize = ESP.getFlashChipSize() / 1024 / 1024;
    uint32_t freeHeap = ESP.getFreeHeap() / 1024;
    uint32_t totalHeap = ESP.getHeapSize() / 1024;
    
    // SD Card info
    uint64_t sdTotal = 0;
    uint64_t sdUsed = 0;
    
    if (SD_MMC.cardType() != CARD_NONE) {
        sdTotal = SD_MMC.totalBytes() / 1024 / 1024;
        sdUsed = SD_MMC.usedBytes() / 1024 / 1024;
    }
    
    snprintf(storage_text, sizeof(storage_text),
             "Flash: %lu MB\n"
             "Free RAM: %lu KB / %lu KB\n"
             "SD Card: %llu MB / %llu MB used",
             flashSize, freeHeap, totalHeap, sdUsed, sdTotal);
    
    lv_label_set_text(storage_label, storage_text);
}

// WiFi switch event
static void wifi_switch_event(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        setWiFi(true);
    } else {
        setWiFi(false);
    }
}

// Bluetooth switch event
static void bt_switch_event(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        setBluetooth(true);
    } else {
        setBluetooth(false);
    }
}

// Brightness slider event
static void brightness_slider_event(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    setScreenBrightness((uint8_t)value);
}

// Create settings menu
void createSettingsMenu() {
    lv_obj_clean(lv_scr_act());
    lv_obj_t * screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF0F0F0), 0);
    
    // Title bar
    lv_obj_t * title_bar = lv_obj_create(screen);
    lv_obj_set_size(title_bar, 320, 45);
    lv_obj_set_pos(title_bar, 0, 0);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x607D8B), 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_radius(title_bar, 0, 0);
    
    lv_obj_t * title = lv_label_create(title_bar);
    lv_label_set_text(title, "⚙ Settings");
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 15, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, &lv_font_montserrat_18);
    lv_obj_add_style(title, &title_style, 0);
    
    // Back button
    lv_obj_t * back_btn = lv_btn_create(title_bar);
    lv_obj_set_size(back_btn, 80, 35);
    lv_obj_align(back_btn, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "BACK");
    lv_obj_center(back_label);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0x607D8B), 0);
    
    lv_obj_add_event_cb(back_btn, [](lv_event_t * e) {
        if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
            // Return to home - adjust based on your EEZ setup
            extern void loadScreen(int screenId);
            loadScreen(1);
        }
    }, LV_EVENT_CLICKED, NULL);
    
    // Content container
    lv_obj_t * content = lv_obj_create(screen);
    lv_obj_set_size(content, 300, 175);
    lv_obj_set_pos(content, 10, 50);
    lv_obj_set_style_bg_color(content, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_pad_all(content, 10, 0);
    
    int y_pos = 10;
    
    // ═══ WiFi Section ═══
    lv_obj_t * wifi_label = lv_label_create(content);
    lv_label_set_text(wifi_label, "WiFi");
    lv_obj_set_pos(wifi_label, 10, y_pos);
    
    wifi_switch = lv_switch_create(content);
    lv_obj_set_pos(wifi_switch, 240, y_pos - 5);
    lv_obj_add_event_cb(wifi_switch, wifi_switch_event, LV_EVENT_VALUE_CHANGED, NULL);
    if (wifiEnabled) lv_obj_add_state(wifi_switch, LV_STATE_CHECKED);
    
    wifi_status_label = lv_label_create(content);
    lv_label_set_text(wifi_status_label, wifiEnabled ? "WiFi: On" : "WiFi: Off");
    lv_obj_set_pos(wifi_status_label, 10, y_pos + 25);
    lv_obj_set_style_text_color(wifi_status_label, lv_color_hex(0x666666), 0);
    
    static lv_style_t small_style;
    lv_style_init(&small_style);
    lv_style_set_text_font(&small_style, &lv_font_montserrat_10);
    lv_obj_add_style(wifi_status_label, &small_style, 0);
    
    y_pos += 50;
    
    // ═══ Bluetooth Section ═══
    lv_obj_t * bt_label = lv_label_create(content);
    lv_label_set_text(bt_label, "Bluetooth");
    lv_obj_set_pos(bt_label, 10, y_pos);
    
    bt_switch = lv_switch_create(content);
    lv_obj_set_pos(bt_switch, 240, y_pos - 5);
    lv_obj_add_event_cb(bt_switch, bt_switch_event, LV_EVENT_VALUE_CHANGED, NULL);
    if (bluetoothEnabled) lv_obj_add_state(bt_switch, LV_STATE_CHECKED);
    
    bt_status_label = lv_label_create(content);
    lv_label_set_text(bt_status_label, bluetoothEnabled ? "BT: On" : "BT: Off");
    lv_obj_set_pos(bt_status_label, 10, y_pos + 25);
    lv_obj_set_style_text_color(bt_status_label, lv_color_hex(0x666666), 0);
    lv_obj_add_style(bt_status_label, &small_style, 0);
    
    y_pos += 50;
    
    // ═══ Brightness Section ═══
    brightness_label = lv_label_create(content);
    char bright_text[32];
    snprintf(bright_text, sizeof(bright_text), "Brightness: %d%%", 
             (screenBrightness * 100) / 255);
    lv_label_set_text(brightness_label, bright_text);
    lv_obj_set_pos(brightness_label, 10, y_pos);
    
    brightness_slider = lv_slider_create(content);
    lv_obj_set_size(brightness_slider, 260, 10);
    lv_obj_set_pos(brightness_slider, 10, y_pos + 25);
    lv_slider_set_range(brightness_slider, 20, 255);
    lv_slider_set_value(brightness_slider, screenBrightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(brightness_slider, brightness_slider_event, 
                       LV_EVENT_VALUE_CHANGED, NULL);
    
    y_pos += 50;
    
    // ═══ Storage Info ═══
    storage_label = lv_label_create(screen);
    lv_obj_set_pos(storage_label, 15, 230);
    lv_obj_set_style_text_color(storage_label, lv_color_hex(0x666666), 0);
    lv_obj_add_style(storage_label, &small_style, 0);
    
    updateStorageInfo();
    
    Serial.println("Settings menu created");
}

// Call this from your EEZ Studio action
void action_open_settings() {
    createSettingsMenu();
}

// Initialize settings (call in setup)
void initSettings() {
    // Load saved settings from SD card if available
    if (SD_MMC.exists("/data/settings.txt")) {
        fs::File file = SD_MMC.open("/data/settings.txt");
        if (file) {
            while (file.available()) {
                String line = file.readStringUntil('\n');
                line.trim();
                
                if (line.startsWith("wifi=")) {
                    wifiEnabled = (line.substring(5) == "1");
                } else if (line.startsWith("bluetooth=")) {
                    bluetoothEnabled = (line.substring(10) == "1");
                } else if (line.startsWith("brightness=")) {
                    screenBrightness = line.substring(11).toInt();
                }
            }
            file.close();
        }
    }
    
    // Apply settings
    if (wifiEnabled) setWiFi(true);
    if (bluetoothEnabled) setBluetooth(true);
    setScreenBrightness(screenBrightness);
    
    Serial.println("Settings initialized");
}

// Save settings to SD card
void saveSettings() {
    fs::File file = SD_MMC.open("/data/settings.txt", FILE_WRITE);
    if (file) {
        file.printf("wifi=%d\n", wifiEnabled ? 1 : 0);
        file.printf("bluetooth=%d\n", bluetoothEnabled ? 1 : 0);
        file.printf("brightness=%d\n", screenBrightness);
        file.close();
        Serial.println("Settings saved");
    }
}

// Auto-save timer callback
void settings_autosave_timer(lv_timer_t * timer) {
    saveSettings();
}

#endif // SETTINGS_MENU_H

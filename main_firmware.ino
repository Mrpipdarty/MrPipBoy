/*
 * Complete Modular System - Main Firmware
 * 
 * This is your minimal core firmware that stays small
 * Apps are stored on SD card and loaded as needed
 * 
 * Flash Usage: ~500KB (vs 1.8MB with all apps compiled in)
 * 
 * Includes:
 * - EEZ Studio UI integration
 * - Modular app loader (apps from SD card)
 * - Settings menu (WiFi/BT/Brightness)
 * - Core drivers only
 */

#include <lvgl.h>
#include <TFT_eSPI.h>
#include "touch.h"
#include "ui.h"
#include "SD_MMC.h"
#include <FS.h>

// Include modular systems
#include "modular_app_loader.h"  // App loader from SD card
#include "settings_menu.h"        // Settings menu

// Debug mode
#define DEBUG_MODE false
#define DEBUG_PRINTLN(x) if(DEBUG_MODE) Serial.println(x)

// SD Card pins
#define SD_CLK   38
#define SD_CMD   40
#define SD_D0    39
#define SD_D1    41
#define SD_D2    48
#define SD_D3    47

// Display
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf[screenWidth*10];

TFT_eSPI my_lcd = TFT_eSPI();

// SD Card status
bool sdCardAvailable = false;

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    my_lcd.setAddrWindow(area->x1, area->y1, w, h);
    my_lcd.pushColors((uint16_t *)&color_p->full, w*h, true);
    lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    if (touch_touched()) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touch_last_x;
        data->point.y = touch_last_y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

bool initSDCard()
{
    if (!SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3)) {
        return false;
    }
    
    delay(50);
    if (!SD_MMC.begin("/sdcard", true)) {
        return false;
    }
    
    if (SD_MMC.cardType() == CARD_NONE) {
        return false;
    }
    
    DEBUG_PRINTLN("SD Card ready");
    return true;
}

void setup()
{
    if (DEBUG_MODE) {
        Serial.begin(115200);
        delay(500);
        Serial.println("\n=== MODULAR SYSTEM BOOT ===");
    }
    
    // Initialize SD Card FIRST
    sdCardAvailable = initSDCard();
    
    if (sdCardAvailable) {
        DEBUG_PRINTLN("✓ SD Card OK");
        
        // Create required directories
        if (!SD_MMC.exists("/apps")) SD_MMC.mkdir("/apps");
        if (!SD_MMC.exists("/data")) SD_MMC.mkdir("/data");
        
        // Initialize modular app system
        initModularAppSystem();
        
        // Initialize settings (loads from SD card)
        initSettings();
    } else {
        DEBUG_PRINTLN("✗ No SD Card - limited functionality");
    }
    
    // Initialize display
    my_lcd.init();
    my_lcd.fillScreen(0xFFFF);
    my_lcd.setRotation(1);
    touch_init(my_lcd.width(), my_lcd.height(), my_lcd.getRotation());
    DEBUG_PRINTLN("✓ Display initialized");
    
    // Initialize LVGL
    lv_init();
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, screenWidth*10);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = my_lcd.width();
    disp_drv.ver_res = my_lcd.height();
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);
    
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
    DEBUG_PRINTLN("✓ LVGL initialized");
    
    // Initialize EEZ Studio UI (your custom home screen)
    ui_init();
    delay(50);
    
    // Print system info
    if (DEBUG_MODE) {
        Serial.println("\n=== SYSTEM INFO ===");
        Serial.printf("Flash: %lu MB\n", ESP.getFlashChipSize() / 1024 / 1024);
        Serial.printf("Free Heap: %lu KB\n", ESP.getFreeHeap() / 1024);
        Serial.printf("PSRAM: %lu KB\n", ESP.getPsramSize() / 1024);
        
        if (sdCardAvailable) {
            Serial.printf("SD Card: %llu MB\n", SD_MMC.cardSize() / 1024 / 1024);
            Serial.printf("Apps Found: %d\n", appCount);
        }
        Serial.println("===================\n");
    }
    
    DEBUG_PRINTLN("✓ System ready");
}

void loop()
{
    lv_task_handler();
    ui_tick();
    
    // Run current app loop if one is active
    // (Apps handle their own loop functions)
    
    delay(5);
}

// ═══════════════════════════════════════════════════════════════
// EEZ STUDIO ACTION FUNCTIONS
// Add these to your actions.c file
// ═══════════════════════════════════════════════════════════════

/*
In your actions.c file, add:

void action_open_apps() {
    action_open_modular_apps();  // Opens app launcher
}

void action_open_settings() {
    action_open_settings();  // Opens settings menu
}

Then in your actions array:

ActionExecFunc actions[] = {
    0,
    action_open_apps,     // Index 1
    action_open_settings, // Index 2
    // ... other actions
};
*/

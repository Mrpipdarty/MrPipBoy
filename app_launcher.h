/*
 * EEZ Studio Integration for App Launcher
 * 
 * This file integrates the app launcher with your EEZ Studio project
 * 
 * SETUP INSTRUCTIONS:
 * 1. Add this code to a new file: app_launcher.h
 * 2. Include it in your main sketch: #include "app_launcher.h"
 * 3. In EEZ Studio, add a button and create an action for it
 * 4. In actions.c, call the appropriate function
 */

#ifndef APP_LAUNCHER_H
#define APP_LAUNCHER_H

#include <lvgl.h>
#include "SD_MMC.h"
#include <FS.h>

// Forward declare EEZ function to return to home screen
extern void loadScreen(enum ScreensEnum screenId);

#define MAX_SD_APPS 20
#define MAX_APP_NAME 32

// App structure for SD card apps
struct SDApp {
    char name[MAX_APP_NAME];
    char description[64];
    char filename[64];
    bool enabled;
};

// Global variables
SDApp sdApps[MAX_SD_APPS];
int sdAppCount = 0;
lv_obj_t * currentAppScreen = NULL;

// ═══════════════════════════════════════════════════════════════
// SD CARD APP SCANNING
// ═══════════════════════════════════════════════════════════════

void scanSDCardApps() {
    sdAppCount = 0;
    
    Serial.println("Scanning /apps/ for applications...");
    
    fs::File root = SD_MMC.open("/apps");
    if (!root || !root.isDirectory()) {
        Serial.println("No /apps directory");
        return;
    }
    
    fs::File file = root.openNextFile();
    while (file && sdAppCount < MAX_SD_APPS) {
        String filename = String(file.name());
        
        if (!file.isDirectory() && filename.endsWith(".app")) {
            // Read app metadata
            fs::File appFile = SD_MMC.open(file.path());
            
            String name = "";
            String desc = "";
            bool enabled = true;
            
            while (appFile.available()) {
                String line = appFile.readStringUntil('\n');
                line.trim();
                
                if (line.startsWith("name=")) {
                    name = line.substring(5);
                } else if (line.startsWith("description=")) {
                    desc = line.substring(12);
                } else if (line.startsWith("enabled=")) {
                    enabled = (line.substring(8) == "true");
                }
            }
            appFile.close();
            
            if (name.length() > 0 && enabled) {
                strncpy(sdApps[sdAppCount].name, name.c_str(), MAX_APP_NAME - 1);
                strncpy(sdApps[sdAppCount].description, desc.c_str(), 63);
                strncpy(sdApps[sdAppCount].filename, file.path(), 63);
                sdApps[sdAppCount].enabled = enabled;
                
                Serial.printf("  Found: %s\n", name.c_str());
                sdAppCount++;
            }
        }
        file = root.openNextFile();
    }
    
    Serial.printf("Found %d apps\n", sdAppCount);
}

// ═══════════════════════════════════════════════════════════════
// APP LAUNCHER UI
// ═══════════════════════════════════════════════════════════════

// Event handler for app launch buttons
static void app_launch_handler(lv_event_t * e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    
    int appIndex = (int)(intptr_t)lv_event_get_user_data(e);
    
    if (appIndex >= 0 && appIndex < sdAppCount) {
        Serial.printf("Launching: %s\n", sdApps[appIndex].name);
        
        // For now, show a message that the app would launch
        // You'll replace this with actual app loading logic
        lv_obj_clean(lv_scr_act());
        
        lv_obj_t * msg = lv_label_create(lv_scr_act());
        char msgText[128];
        snprintf(msgText, sizeof(msgText), "App: %s\n\n%s\n\nFunctionality coming soon!", 
                 sdApps[appIndex].name, sdApps[appIndex].description);
        lv_label_set_text(msg, msgText);
        lv_obj_center(msg);
        lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
        
        // Add back button
        lv_obj_t * back_btn = lv_btn_create(lv_scr_act());
        lv_obj_set_size(back_btn, 120, 50);
        lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
        
        lv_obj_t * back_label = lv_label_create(back_btn);
        lv_label_set_text(back_label, "BACK");
        lv_obj_center(back_label);
        
        lv_obj_add_event_cb(back_btn, [](lv_event_t * e) {
            if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
                // Return to app launcher
                extern void createAppLauncher();
                createAppLauncher();
            }
        }, LV_EVENT_CLICKED, NULL);
    }
}

// Event handler for home button
static void home_button_handler(lv_event_t * e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    
    // Return to EEZ Studio home screen
    lv_obj_clean(lv_scr_act());
    
    // Load your main EEZ screen (adjust screen ID based on your project)
    extern void loadScreen(int screenId);
    loadScreen(1);  // Typically screen ID 1 is the main/home screen
}

// Create the app launcher UI
void createAppLauncher() {
    Serial.println("Creating app launcher...");
    
    // Rescan apps each time launcher opens
    scanSDCardApps();
    
    // Clear the screen
    lv_obj_clean(lv_scr_act());
    lv_obj_t * screen = lv_scr_act();
    
    // Set background color
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF0F0F0), 0);
    
    // Title bar
    lv_obj_t * title_bar = lv_obj_create(screen);
    lv_obj_set_size(title_bar, 320, 50);
    lv_obj_set_pos(title_bar, 0, 0);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_radius(title_bar, 0, 0);
    
    lv_obj_t * title = lv_label_create(title_bar);
    lv_label_set_text(title, "Applications");
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 15, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, &lv_font_montserrat_20);
    lv_obj_add_style(title, &title_style, 0);
    
    // Home button in title bar
    lv_obj_t * home_btn = lv_btn_create(title_bar);
    lv_obj_set_size(home_btn, 80, 35);
    lv_obj_align(home_btn, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(home_btn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_add_event_cb(home_btn, home_button_handler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * home_label = lv_label_create(home_btn);
    lv_label_set_text(home_label, "HOME");
    lv_obj_center(home_label);
    lv_obj_set_style_text_color(home_label, lv_color_hex(0x2196F3), 0);
    
    // Scrollable container for apps
    lv_obj_t * scroll_container = lv_obj_create(screen);
    lv_obj_set_size(scroll_container, 300, 165);
    lv_obj_set_pos(scroll_container, 10, 60);
    lv_obj_set_style_pad_all(scroll_container, 5, 0);
    lv_obj_set_style_bg_color(scroll_container, lv_color_hex(0xFFFFFF), 0);
    
    if (sdAppCount == 0) {
        // No apps found message
        lv_obj_t * no_apps = lv_label_create(scroll_container);
        lv_label_set_text(no_apps, 
            "No apps found\n\n"
            "Add .app files to\n"
            "/apps/ folder on SD card");
        lv_obj_center(no_apps);
        lv_obj_set_style_text_align(no_apps, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(no_apps, lv_color_hex(0x888888), 0);
    } else {
        // Create app list
        for (int i = 0; i < sdAppCount; i++) {
            lv_obj_t * app_container = lv_btn_create(scroll_container);
            lv_obj_set_size(app_container, 280, 60);
            lv_obj_set_pos(app_container, 5, i * 65);
            lv_obj_set_style_bg_color(app_container, lv_color_hex(0xF8F8F8), 0);
            lv_obj_set_style_border_width(app_container, 2, 0);
            lv_obj_set_style_border_color(app_container, lv_color_hex(0xE0E0E0), 0);
            lv_obj_add_event_cb(app_container, app_launch_handler, LV_EVENT_CLICKED, (void*)(intptr_t)i);
            
            // App icon placeholder
            lv_obj_t * icon = lv_obj_create(app_container);
            lv_obj_set_size(icon, 40, 40);
            lv_obj_align(icon, LV_ALIGN_LEFT_MID, 5, 0);
            lv_obj_set_style_bg_color(icon, lv_color_hex(0x2196F3), 0);
            lv_obj_set_style_radius(icon, 5, 0);
            
            // App name
            lv_obj_t * app_name = lv_label_create(app_container);
            lv_label_set_text(app_name, sdApps[i].name);
            lv_obj_align(app_name, LV_ALIGN_TOP_LEFT, 55, 8);
            
            static lv_style_t name_style;
            lv_style_init(&name_style);
            lv_style_set_text_font(&name_style, &lv_font_montserrat_14);
            lv_obj_add_style(app_name, &name_style, 0);
            
            // App description
            lv_obj_t * app_desc = lv_label_create(app_container);
            lv_label_set_text(app_desc, sdApps[i].description);
            lv_obj_align(app_desc, LV_ALIGN_BOTTOM_LEFT, 55, -8);
            
            static lv_style_t desc_style;
            lv_style_init(&desc_style);
            lv_style_set_text_font(&desc_style, &lv_font_montserrat_10);
            lv_style_set_text_color(&desc_style, lv_color_hex(0x666666));
            lv_obj_add_style(app_desc, &desc_style, 0);
        }
    }
    
    // Footer info
    lv_obj_t * footer = lv_label_create(screen);
    char footer_text[64];
    snprintf(footer_text, sizeof(footer_text), "%d app%s available", 
             sdAppCount, sdAppCount == 1 ? "" : "s");
    lv_label_set_text(footer, footer_text);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_text_color(footer, lv_color_hex(0x888888), 0);
    
    Serial.println("App launcher created");
}

// ═══════════════════════════════════════════════════════════════
// FUNCTIONS TO CALL FROM EEZ STUDIO ACTIONS
// ═══════════════════════════════════════════════════════════════

// Call this from your EEZ Studio "Apps" button
void action_open_app_launcher() {
    Serial.println("Opening app launcher...");
    createAppLauncher();
}

// Initialize app system (call in setup)
void initAppLauncher() {
    // Ensure /apps directory exists
    if (!SD_MMC.exists("/apps")) {
        SD_MMC.mkdir("/apps");
        Serial.println("Created /apps directory");
    }
    
    // Initial scan
    scanSDCardApps();
}

#endif // APP_LAUNCHER_H

/*
 * Modular App Loader System
 * 
 * This system allows you to store large app implementations on SD card
 * as separate .h header files, which are loaded into RAM when needed.
 * 
 * This frees up flash memory on the ESP32-S3
 * 
 * File: modular_app_loader.h
 */

#ifndef MODULAR_APP_LOADER_H
#define MODULAR_APP_LOADER_H

#include <lvgl.h>
#include "SD_MMC.h"
#include <FS.h>

#define MAX_APPS 20
#define MAX_APP_NAME 32
#define MAX_APP_CODE_SIZE 50000  // 50KB max per app

// App function pointer types
typedef void (*AppSetupFunc)();
typedef void (*AppLoopFunc)();
typedef void (*AppCleanupFunc)();

// App structure
struct ModularApp {
    char name[MAX_APP_NAME];
    char description[64];
    char filepath[64];
    bool enabled;
    bool loaded;
    
    // Function pointers
    AppSetupFunc setup;
    AppLoopFunc loop;
    AppCleanupFunc cleanup;
    
    // Code storage
    char* codeBuffer;
    size_t codeSize;
};

// Global app registry
ModularApp appRegistry[MAX_APPS];
int appCount = 0;
int currentAppIndex = -1;

// ═══════════════════════════════════════════════════════════════
// APP LOADING SYSTEM
// ═══════════════════════════════════════════════════════════════

// Scan SD card for app manifest files
void scanForModularApps() {
    appCount = 0;
    
    Serial.println("\n=== Scanning for Modular Apps ===");
    
    fs::File root = SD_MMC.open("/apps");
    if (!root || !root.isDirectory()) {
        Serial.println("No /apps directory");
        return;
    }
    
    fs::File file = root.openNextFile();
    while (file && appCount < MAX_APPS) {
        String filename = String(file.name());
        
        // Look for .manifest files
        if (!file.isDirectory() && filename.endsWith(".manifest")) {
            Serial.printf("Found manifest: %s\n", filename.c_str());
            
            // Read manifest
            fs::File manifest = SD_MMC.open(file.path());
            
            String name = "";
            String desc = "";
            String codefile = "";
            bool enabled = true;
            
            while (manifest.available()) {
                String line = manifest.readStringUntil('\n');
                line.trim();
                
                if (line.startsWith("name=")) {
                    name = line.substring(5);
                } else if (line.startsWith("description=")) {
                    desc = line.substring(12);
                } else if (line.startsWith("codefile=")) {
                    codefile = line.substring(9);
                } else if (line.startsWith("enabled=")) {
                    enabled = (line.substring(8) == "true");
                }
            }
            manifest.close();
            
            if (name.length() > 0 && codefile.length() > 0 && enabled) {
                // Register app
                strncpy(appRegistry[appCount].name, name.c_str(), MAX_APP_NAME - 1);
                strncpy(appRegistry[appCount].description, desc.c_str(), 63);
                
                // Build full path to code file
                char fullpath[64];
                snprintf(fullpath, sizeof(fullpath), "/apps/%s", codefile.c_str());
                strncpy(appRegistry[appCount].filepath, fullpath, 63);
                
                appRegistry[appCount].enabled = enabled;
                appRegistry[appCount].loaded = false;
                appRegistry[appCount].codeBuffer = NULL;
                appRegistry[appCount].setup = NULL;
                appRegistry[appCount].loop = NULL;
                appRegistry[appCount].cleanup = NULL;
                
                Serial.printf("  Registered: %s -> %s\n", name.c_str(), fullpath);
                appCount++;
            }
        }
        file = root.openNextFile();
    }
    
    Serial.printf("Found %d modular apps\n", appCount);
}

// Load app code from SD card into RAM
bool loadAppCode(int appIndex) {
    if (appIndex < 0 || appIndex >= appCount) return false;
    if (appRegistry[appIndex].loaded) return true;  // Already loaded
    
    Serial.printf("Loading app: %s\n", appRegistry[appIndex].name);
    
    // Open code file
    fs::File file = SD_MMC.open(appRegistry[appIndex].filepath);
    if (!file) {
        Serial.printf("Failed to open: %s\n", appRegistry[appIndex].filepath);
        return false;
    }
    
    size_t fileSize = file.size();
    
    if (fileSize > MAX_APP_CODE_SIZE) {
        Serial.printf("App too large: %d bytes (max %d)\n", fileSize, MAX_APP_CODE_SIZE);
        file.close();
        return false;
    }
    
    // Allocate memory for code
    appRegistry[appIndex].codeBuffer = (char*)malloc(fileSize + 1);
    if (!appRegistry[appIndex].codeBuffer) {
        Serial.println("Failed to allocate memory");
        file.close();
        return false;
    }
    
    // Read code into buffer
    file.read((uint8_t*)appRegistry[appIndex].codeBuffer, fileSize);
    appRegistry[appIndex].codeBuffer[fileSize] = '\0';
    appRegistry[appIndex].codeSize = fileSize;
    file.close();
    
    appRegistry[appIndex].loaded = true;
    
    Serial.printf("Loaded %d bytes into RAM\n", fileSize);
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    
    return true;
}

// Unload app code from RAM
void unloadAppCode(int appIndex) {
    if (appIndex < 0 || appIndex >= appCount) return;
    if (!appRegistry[appIndex].loaded) return;
    
    if (appRegistry[appIndex].codeBuffer) {
        free(appRegistry[appIndex].codeBuffer);
        appRegistry[appIndex].codeBuffer = NULL;
    }
    
    appRegistry[appIndex].loaded = false;
    appRegistry[appIndex].setup = NULL;
    appRegistry[appIndex].loop = NULL;
    appRegistry[appIndex].cleanup = NULL;
    
    Serial.printf("Unloaded app: %s\n", appRegistry[appIndex].name);
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
}

// ═══════════════════════════════════════════════════════════════
// APP LAUNCHER UI
// ═══════════════════════════════════════════════════════════════

void launchModularApp(int appIndex);

static void app_button_event(lv_event_t * e) {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    
    int appIndex = (int)(intptr_t)lv_event_get_user_data(e);
    launchModularApp(appIndex);
}

void createModularAppLauncher() {
    Serial.println("Creating modular app launcher...");
    
    // Rescan apps
    scanForModularApps();
    
    lv_obj_clean(lv_scr_act());
    lv_obj_t * screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF0F0F0), 0);
    
    // Title bar
    lv_obj_t * title_bar = lv_obj_create(screen);
    lv_obj_set_size(title_bar, 320, 50);
    lv_obj_set_pos(title_bar, 0, 0);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_radius(title_bar, 0, 0);
    
    lv_obj_t * title = lv_label_create(title_bar);
    lv_label_set_text(title, "📱 Applications");
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 15, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    
    static lv_style_t title_style;
    lv_style_init(&title_style);
    lv_style_set_text_font(&title_style, &lv_font_montserrat_20);
    lv_obj_add_style(title, &title_style, 0);
    
    // Home button
    lv_obj_t * home_btn = lv_btn_create(title_bar);
    lv_obj_set_size(home_btn, 80, 35);
    lv_obj_align(home_btn, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_style_bg_color(home_btn, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * home_label = lv_label_create(home_btn);
    lv_label_set_text(home_label, "HOME");
    lv_obj_center(home_label);
    lv_obj_set_style_text_color(home_label, lv_color_hex(0x2196F3), 0);
    
    lv_obj_add_event_cb(home_btn, [](lv_event_t * e) {
        if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
            extern void loadScreen(int screenId);
            loadScreen(1);
        }
    }, LV_EVENT_CLICKED, NULL);
    
    // App list container
    lv_obj_t * container = lv_obj_create(screen);
    lv_obj_set_size(container, 300, 165);
    lv_obj_set_pos(container, 10, 60);
    lv_obj_set_style_bg_color(container, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_pad_all(container, 5, 0);
    
    if (appCount == 0) {
        lv_obj_t * no_apps = lv_label_create(container);
        lv_label_set_text(no_apps, 
            "No apps found\n\n"
            "Add apps to /apps/ on SD card");
        lv_obj_center(no_apps);
        lv_obj_set_style_text_align(no_apps, LV_TEXT_ALIGN_CENTER, 0);
    } else {
        for (int i = 0; i < appCount && i < 3; i++) {  // Show max 3 apps
            lv_obj_t * app_btn = lv_btn_create(container);
            lv_obj_set_size(app_btn, 280, 50);
            lv_obj_set_pos(app_btn, 5, i * 55);
            lv_obj_set_style_bg_color(app_btn, lv_color_hex(0xF8F8F8), 0);
            lv_obj_add_event_cb(app_btn, app_button_event, LV_EVENT_CLICKED, 
                               (void*)(intptr_t)i);
            
            // App name
            lv_obj_t * app_name = lv_label_create(app_btn);
            lv_label_set_text(app_name, appRegistry[i].name);
            lv_obj_align(app_name, LV_ALIGN_TOP_LEFT, 10, 8);
            
            // App description
            lv_obj_t * app_desc = lv_label_create(app_btn);
            lv_label_set_text(app_desc, appRegistry[i].description);
            lv_obj_align(app_desc, LV_ALIGN_BOTTOM_LEFT, 10, -8);
            
            static lv_style_t desc_style;
            lv_style_init(&desc_style);
            lv_style_set_text_font(&desc_style, &lv_font_montserrat_10);
            lv_style_set_text_color(&desc_style, lv_color_hex(0x666666));
            lv_obj_add_style(app_desc, &desc_style, 0);
            
            // Size indicator
            if (appRegistry[i].loaded) {
                lv_obj_t * loaded_ind = lv_label_create(app_btn);
                lv_label_set_text(loaded_ind, "●");
                lv_obj_align(loaded_ind, LV_ALIGN_RIGHT_MID, -10, 0);
                lv_obj_set_style_text_color(loaded_ind, lv_color_hex(0x4CAF50), 0);
            }
        }
    }
    
    // Footer
    lv_obj_t * footer = lv_label_create(screen);
    char footer_text[64];
    uint32_t freeHeap = ESP.getFreeHeap() / 1024;
    snprintf(footer_text, sizeof(footer_text), 
             "%d apps | Free RAM: %lu KB", appCount, freeHeap);
    lv_label_set_text(footer, footer_text);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_text_color(footer, lv_color_hex(0x888888), 0);
}

// Launch an app
void launchModularApp(int appIndex) {
    if (appIndex < 0 || appIndex >= appCount) return;
    
    Serial.printf("Launching: %s\n", appRegistry[appIndex].name);
    
    // Load app code if not loaded
    if (!loadAppCode(appIndex)) {
        Serial.println("Failed to load app");
        return;
    }
    
    // For now, show that the app is loaded
    // In a real implementation, you would parse and execute the code
    currentAppIndex = appIndex;
    
    lv_obj_clean(lv_scr_act());
    lv_obj_t * screen = lv_scr_act();
    
    lv_obj_t * msg = lv_label_create(screen);
    char msgText[256];
    snprintf(msgText, sizeof(msgText),
             "App: %s\n\n"
             "%s\n\n"
             "Code loaded: %d bytes\n"
             "Free RAM: %d KB\n\n"
             "App execution coming soon!",
             appRegistry[appIndex].name,
             appRegistry[appIndex].description,
             appRegistry[appIndex].codeSize,
             ESP.getFreeHeap() / 1024);
    lv_label_set_text(msg, msgText);
    lv_obj_center(msg);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    
    // Back button
    lv_obj_t * back_btn = lv_btn_create(screen);
    lv_obj_set_size(back_btn, 120, 50);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    lv_obj_t * back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "BACK");
    lv_obj_center(back_label);
    
    lv_obj_add_event_cb(back_btn, [](lv_event_t * e) {
        if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
            // Unload current app
            if (currentAppIndex >= 0) {
                unloadAppCode(currentAppIndex);
                currentAppIndex = -1;
            }
            createModularAppLauncher();
        }
    }, LV_EVENT_CLICKED, NULL);
}

// Call from EEZ Studio action
void action_open_modular_apps() {
    createModularAppLauncher();
}

// Initialize system
void initModularAppSystem() {
    scanForModularApps();
    Serial.println("Modular app system initialized");
}

#endif // MODULAR_APP_LOADER_H

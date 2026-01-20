/*
 * EEZ-SD Bridge Helper
 * 
 * This helper makes it easy to connect EEZ Studio widgets
 * with app functions stored on SD card
 * 
 * Add this to your project as: eez_sd_bridge.h
 */

#ifndef EEZ_SD_BRIDGE_H
#define EEZ_SD_BRIDGE_H

#include <lvgl.h>
#include "ui.h"

// ═══════════════════════════════════════════════════════════════
// WIDGET FINDER - Get EEZ widgets by name/type
// ═══════════════════════════════════════════════════════════════

// Find a label widget on current screen by searching
lv_obj_t * find_label_by_text(const char* text) {
    lv_obj_t * screen = lv_scr_act();
    
    // Iterate through all children
    uint32_t child_cnt = lv_obj_get_child_cnt(screen);
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t * child = lv_obj_get_child(screen, i);
        
        // Check if it's a label
        if(lv_obj_check_type(child, &lv_label_class)) {
            const char * label_text = lv_label_get_text(child);
            if(strcmp(label_text, text) == 0) {
                return child;
            }
        }
        
        // Search recursively
        uint32_t subchild_cnt = lv_obj_get_child_cnt(child);
        for(uint32_t j = 0; j < subchild_cnt; j++) {
            lv_obj_t * subchild = lv_obj_get_child(child, j);
            if(lv_obj_check_type(subchild, &lv_label_class)) {
                const char * label_text = lv_label_get_text(subchild);
                if(strcmp(label_text, text) == 0) {
                    return subchild;
                }
            }
        }
    }
    
    return NULL;
}

// Store reference to frequently used widgets
static lv_obj_t * cached_widgets[20];
static int cached_widget_count = 0;

void cache_widget(lv_obj_t * widget) {
    if (cached_widget_count < 20) {
        cached_widgets[cached_widget_count++] = widget;
    }
}

void clear_widget_cache() {
    cached_widget_count = 0;
}

// ═══════════════════════════════════════════════════════════════
// SCREEN MANAGEMENT
// ═══════════════════════════════════════════════════════════════

// Current app info
static int currentAppScreenId = -1;
static char currentAppName[32] = "";

void set_current_app(const char* app_name, int screen_id) {
    strncpy(currentAppName, app_name, 31);
    currentAppScreenId = screen_id;
    clear_widget_cache();
    
    Serial.printf("Current app: %s (Screen %d)\n", app_name, screen_id);
}

const char* get_current_app_name() {
    return currentAppName;
}

int get_current_app_screen_id() {
    return currentAppScreenId;
}

// ═══════════════════════════════════════════════════════════════
// ACTION ROUTER - Route EEZ actions to SD card functions
// ═══════════════════════════════════════════════════════════════

// Function pointer types for SD card app functions
typedef void (*SDAppEventHandler)(lv_event_t * e);
typedef void (*SDAppButtonHandler)(const char* button_data);
typedef void (*SDAppVoidHandler)();

// Action router for button clicks
void route_to_sd_app(lv_event_t * e, SDAppButtonHandler handler) {
    if (!handler) return;
    
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;
    
    // Get button label text
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    
    if (label && lv_obj_check_type(label, &lv_label_class)) {
        const char * text = lv_label_get_text(label);
        handler(text);
    }
}

// ═══════════════════════════════════════════════════════════════
// SIMPLIFIED ACTION HANDLERS
// ═══════════════════════════════════════════════════════════════

// Example: Generic button handler that routes to SD app
void generic_app_button_handler(lv_event_t * e) {
    if (strcmp(currentAppName, "Calculator") == 0) {
        extern void calc_handle_button(const char* btn);
        route_to_sd_app(e, calc_handle_button);
    }
    else if (strcmp(currentAppName, "Keyboard") == 0) {
        extern void keyboard_handle_button(const char* btn);
        route_to_sd_app(e, keyboard_handle_button);
    }
    // Add more apps as needed
}

// ═══════════════════════════════════════════════════════════════
// EEZ STUDIO INTEGRATION HELPERS
// ═══════════════════════════════════════════════════════════════

// Call this when an EEZ screen loads
void on_screen_loaded(int screen_id) {
    Serial.printf("Screen loaded: %d\n", screen_id);
    
    // Initialize app-specific widgets
    if (screen_id == currentAppScreenId) {
        // Screen matches current app
        // App can now safely access widgets
    }
}

// Update a label by searching for it
void update_label(const char* search_text, const char* new_text) {
    lv_obj_t * label = find_label_by_text(search_text);
    if (label) {
        lv_label_set_text(label, new_text);
    }
}

// ═══════════════════════════════════════════════════════════════
// EXAMPLE USAGE IN ACTIONS.C
// ═══════════════════════════════════════════════════════════════

/*

// In actions.c:

#include "eez_sd_bridge.h"

// When calculator app launches
void action_launch_calculator() {
    set_current_app("Calculator", 5);  // Screen ID 5
    loadScreen(5);
    
    extern void calculator_app_setup();
    calculator_app_setup();
}

// Calculator number buttons (0-9, all use same action)
void action_calc_number(lv_event_t * e) {
    extern void calc_handle_number(const char* digit);
    route_to_sd_app(e, calc_handle_number);
}

// Calculator operation buttons (+, -, *, /, =)
void action_calc_operation(lv_event_t * e) {
    extern void calc_handle_operation_str(const char* op);
    route_to_sd_app(e, calc_handle_operation_str);
}

// Or use the generic handler
void action_calc_button(lv_event_t * e) {
    generic_app_button_handler(e);
}

*/

// ═══════════════════════════════════════════════════════════════
// EXAMPLE SD CARD APP USING BRIDGE
// ═══════════════════════════════════════════════════════════════

/*

// In /apps/calculator_app.h on SD card:

// Calculator uses the bridge to update display
void calc_handle_button(const char* btn) {
    static char display[32] = "0";
    
    // Process button
    if (strcmp(btn, "C") == 0) {
        strcpy(display, "0");
    } else if (strcmp(btn, "=") == 0) {
        // Calculate result
        // ... calculation logic ...
    } else {
        // Append digit
        if (strcmp(display, "0") == 0) {
            strcpy(display, btn);
        } else {
            strcat(display, btn);
        }
    }
    
    // Update display using bridge helper
    extern void update_label(const char*, const char*);
    update_label("0", display);  // Find label showing "0" and update it
}

*/

// ═══════════════════════════════════════════════════════════════
// DEBUGGING HELPERS
// ═══════════════════════════════════════════════════════════════

void debug_print_screen_widgets() {
    lv_obj_t * screen = lv_scr_act();
    Serial.println("\n=== Current Screen Widgets ===");
    
    uint32_t child_cnt = lv_obj_get_child_cnt(screen);
    Serial.printf("Total widgets: %d\n", child_cnt);
    
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t * child = lv_obj_get_child(screen, i);
        
        if(lv_obj_check_type(child, &lv_label_class)) {
            const char * text = lv_label_get_text(child);
            Serial.printf("  Label %d: \"%s\"\n", i, text);
        } else if(lv_obj_check_type(child, &lv_btn_class)) {
            Serial.printf("  Button %d\n", i);
        } else {
            Serial.printf("  Widget %d (other type)\n", i);
        }
    }
    Serial.println("==============================\n");
}

#endif // EEZ_SD_BRIDGE_H

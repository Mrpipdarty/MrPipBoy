#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

void action_open_apps() {
    action_open_modular_apps();  // Opens app launcher
}

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/

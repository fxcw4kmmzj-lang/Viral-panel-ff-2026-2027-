/*
 * AdvancedPanel — Advanced in-game panel GUI, C ABI.
 *
 * Copyright (c) 2026 Sotos. All rights reserved.
 * This file is part of the AdvancedPanel project. See LICENSE.
 *
 * The C ABI is the stable contract. Objective-C, C# and C++ hosts all
 * bind to these symbols, and the version is pinned by AP_API_VERSION.
 */

#ifndef ADVANCED_PANEL_H
#define ADVANCED_PANEL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#  define AP_API __declspec(dllexport)
#else
#  define AP_API __attribute__((visibility("default")))
#endif

#define AP_API_VERSION 1

typedef struct APContext APContext;

typedef enum APTheme {
    AP_THEME_DARK  = 0,
    AP_THEME_LIGHT = 1
} APTheme;

typedef enum APWidgetKind {
    AP_WIDGET_TOGGLE = 0,
    AP_WIDGET_SLIDER = 1,
    AP_WIDGET_BUTTON = 2,
    AP_WIDGET_LABEL  = 3
} APWidgetKind;

typedef enum APEventKind {
    AP_EVENT_CLICK         = 1,
    AP_EVENT_VALUE_CHANGED = 2
} APEventKind;

typedef enum APResult {
    AP_OK                 =  0,
    AP_ERR_INVALID_ARG    = -1,
    AP_ERR_NOT_FOUND      = -2,
    AP_ERR_ALREADY_EXISTS = -3,
    AP_ERR_WRONG_KIND     = -4,
    AP_ERR_OUT_OF_RANGE   = -5
} APResult;

typedef struct APPanelConfig {
    const char *title;
    const char *version;
    const char *author;
    int         width;
    int         height;
    APTheme     theme;
    unsigned    flags;
} APPanelConfig;

typedef struct APWidgetInfo {
    const char  *tab_id;
    const char  *id;
    APWidgetKind kind;
    const char  *label;
    double       min_value;
    double       max_value;
    double       value;
    int          enabled;
} APWidgetInfo;

/* Fired on the thread that calls ap_dispatch_event(). */
typedef void (*APEventCallback)(void *user_data,
                                const char *tab_id,
                                const char *widget_id,
                                APEventKind kind,
                                double value);

AP_API const char *ap_version(void);
AP_API int         ap_api_version(void);
AP_API const char *ap_build_info(void);

AP_API APContext *ap_create(const APPanelConfig *config);
AP_API void       ap_destroy(APContext *ctx);

AP_API int ap_add_tab(APContext *ctx, const char *tab_id, const char *title);
AP_API int ap_remove_tab(APContext *ctx, const char *tab_id);
AP_API int ap_tab_count(APContext *ctx);

AP_API int ap_add_toggle(APContext *ctx, const char *tab_id, const char *widget_id,
                         const char *label, int initial_value);
AP_API int ap_add_slider(APContext *ctx, const char *tab_id, const char *widget_id,
                         const char *label, double min_value, double max_value,
                         double initial_value);
AP_API int ap_add_button(APContext *ctx, const char *tab_id, const char *widget_id,
                         const char *label);
AP_API int ap_add_label(APContext *ctx, const char *tab_id, const char *widget_id,
                        const char *text);

AP_API int    ap_get_toggle(APContext *ctx, const char *widget_id);
AP_API int    ap_set_toggle(APContext *ctx, const char *widget_id, int value);
AP_API double ap_get_slider(APContext *ctx, const char *widget_id);
AP_API int    ap_set_slider(APContext *ctx, const char *widget_id, double value);

AP_API void ap_set_callback(APContext *ctx, APEventCallback cb, void *user_data);
AP_API int  ap_dispatch_event(APContext *ctx, const char *tab_id,
                              const char *widget_id, APEventKind kind, double value);

AP_API int ap_widget_count(APContext *ctx);
AP_API int ap_widget_at(APContext *ctx, int index, APWidgetInfo *out);

/* Returns a malloc'd JSON snapshot. Release with ap_free_string(). */
AP_API const char *ap_serialize(APContext *ctx);
AP_API void        ap_free_string(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* ADVANCED_PANEL_H */

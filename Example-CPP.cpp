// AdvancedPanel — C++ host example.
// Copyright (c) 2026 Sotos. All rights reserved. See LICENSE.

#include "advanced_panel.h"

#include <cstdio>

static void on_event(void *, const char *tab, const char *widget, APEventKind kind, double value) {
    std::printf("[event] tab=%s widget=%s kind=%d value=%.3f\n",
                tab ? tab : "", widget ? widget : "", (int)kind, value);
}

int main() {
    std::printf("%s\n", ap_build_info());

    APPanelConfig cfg{};
    cfg.title   = "Advanced Panel";
    cfg.version = "1.0.0";
    cfg.author  = "Sotos";
    cfg.width   = 1280;
    cfg.height  = 720;
    cfg.theme   = AP_THEME_DARK;

    APContext *ctx = ap_create(&cfg);
    if (!ctx) return 1;

    ap_add_tab(ctx, "graphics", "Graphics");
    ap_add_tab(ctx, "audio", "Audio");

    ap_add_toggle(ctx, "graphics", "vsync", "V-Sync", 1);
    ap_add_slider(ctx, "graphics", "render_scale", "Render Scale", 0.5, 2.0, 1.0);
    ap_add_toggle(ctx, "graphics", "shadows", "Shadows", 1);
    ap_add_slider(ctx, "audio", "master", "Master Volume", 0.0, 1.0, 0.8);
    ap_add_button(ctx, "audio", "reset", "Reset to Defaults");

    ap_set_callback(ctx, on_event, nullptr);

    ap_dispatch_event(ctx, "graphics", "vsync", AP_EVENT_CLICK, 0.0);
    ap_dispatch_event(ctx, "graphics", "render_scale", AP_EVENT_VALUE_CHANGED, 1.25);
    ap_dispatch_event(ctx, "audio", "reset", AP_EVENT_CLICK, 0.0);

    const char *json = ap_serialize(ctx);
    std::printf("%s\n", json ? json : "(null)");
    ap_free_string(json);

    ap_destroy(ctx);
    return 0;
}

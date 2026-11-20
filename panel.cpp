// AdvancedPanel — core implementation.
// Copyright (c) 2026 Sotos. All rights reserved. See LICENSE.

#include "advanced_panel.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr const char *kVersion   = "1.0.0";
constexpr const char *kBuildDate = __DATE__;

struct Widget {
    std::string  tab_id;
    std::string  id;
    std::string  label;
    APWidgetKind kind;
    double       min_value;
    double       max_value;
    double       value;
    bool         enabled;
};

struct Tab {
    std::string id;
    std::string title;
};

} // namespace

struct APContext {
    std::mutex              mutex;
    APPanelConfig           config{};
    std::string             title;
    std::string             version;
    std::string             author;
    std::vector<Tab>        tabs;
    std::vector<Widget>     widgets;
    APEventCallback         callback  = nullptr;
    void                   *user_data = nullptr;
};

namespace {

// Mutex is held by every caller below; these helpers assume that.
APContext *valid(APContext *ctx) { return ctx; }

Widget *find_widget(APContext *ctx, const char *widget_id) {
    if (!ctx || !widget_id) return nullptr;
    for (auto &w : ctx->widgets) {
        if (w.id == widget_id) return &w;
    }
    return nullptr;
}

bool has_tab(APContext *ctx, const char *tab_id) {
    if (!ctx || !tab_id) return false;
    return std::any_of(ctx->tabs.begin(), ctx->tabs.end(),
                       [&](const Tab &t) { return t.id == tab_id; });
}

std::string escape_json(const std::string &in) {
    std::string out;
    out.reserve(in.size() + 8);
    for (char c : in) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

const char *kind_name(APWidgetKind kind) {
    switch (kind) {
        case AP_WIDGET_TOGGLE: return "toggle";
        case AP_WIDGET_SLIDER: return "slider";
        case AP_WIDGET_BUTTON: return "button";
        case AP_WIDGET_LABEL:  return "label";
    }
    return "unknown";
}

} // namespace

extern "C" {

const char *ap_version(void) { return kVersion; }

int ap_api_version(void) { return AP_API_VERSION; }

const char *ap_build_info(void) {
    static char info[128];
    static std::once_flag once;
    std::call_once(once, [] {
        std::snprintf(info, sizeof(info), "AdvancedPanel %s (api %d, built %s)",
                      kVersion, AP_API_VERSION, kBuildDate);
    });
    return info;
}

APContext *ap_create(const APPanelConfig *config) {
    if (!config) return nullptr;
    auto *ctx = new (std::nothrow) APContext();
    if (!ctx) return nullptr;

    ctx->config = *config;
    ctx->title   = config->title   ? config->title   : "Advanced Panel";
    ctx->version = config->version ? config->version : "0.0.0";
    ctx->author  = config->author  ? config->author  : "";
    return ctx;
}

void ap_destroy(APContext *ctx) { delete ctx; }

int ap_add_tab(APContext *ctx, const char *tab_id, const char *title) {
    if (!valid(ctx) || !tab_id || !*tab_id) return AP_ERR_INVALID_ARG;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    if (has_tab(ctx, tab_id)) return AP_ERR_ALREADY_EXISTS;
    ctx->tabs.push_back({tab_id, title ? title : tab_id});
    return AP_OK;
}

int ap_remove_tab(APContext *ctx, const char *tab_id) {
    if (!valid(ctx) || !tab_id) return AP_ERR_INVALID_ARG;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    auto it = std::find_if(ctx->tabs.begin(), ctx->tabs.end(),
                           [&](const Tab &t) { return t.id == tab_id; });
    if (it == ctx->tabs.end()) return AP_ERR_NOT_FOUND;
    ctx->tabs.erase(it);
    ctx->widgets.erase(std::remove_if(ctx->widgets.begin(), ctx->widgets.end(),
                                      [&](const Widget &w) { return w.tab_id == tab_id; }),
                       ctx->widgets.end());
    return AP_OK;
}

int ap_tab_count(APContext *ctx) {
    if (!valid(ctx)) return 0;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    return static_cast<int>(ctx->tabs.size());
}

static int add_widget(APContext *ctx, const char *tab_id, const char *widget_id,
                      const char *label, APWidgetKind kind,
                      double minv, double maxv, double value) {
    if (!valid(ctx) || !tab_id || !widget_id || !*widget_id) return AP_ERR_INVALID_ARG;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    if (!has_tab(ctx, tab_id)) return AP_ERR_NOT_FOUND;
    if (find_widget(ctx, widget_id)) return AP_ERR_ALREADY_EXISTS;
    Widget w;
    w.tab_id    = tab_id;
    w.id        = widget_id;
    w.label     = label ? label : widget_id;
    w.kind      = kind;
    w.min_value = minv;
    w.max_value = maxv;
    w.value     = value;
    w.enabled   = true;
    ctx->widgets.push_back(std::move(w));
    return AP_OK;
}

int ap_add_toggle(APContext *ctx, const char *tab_id, const char *widget_id,
                  const char *label, int initial_value) {
    return add_widget(ctx, tab_id, widget_id, label, AP_WIDGET_TOGGLE,
                      0.0, 1.0, initial_value ? 1.0 : 0.0);
}

int ap_add_slider(APContext *ctx, const char *tab_id, const char *widget_id,
                  const char *label, double min_value, double max_value,
                  double initial_value) {
    if (min_value > max_value) return AP_ERR_OUT_OF_RANGE;
    double clamped = std::min(std::max(initial_value, min_value), max_value);
    return add_widget(ctx, tab_id, widget_id, label, AP_WIDGET_SLIDER,
                      min_value, max_value, clamped);
}

int ap_add_button(APContext *ctx, const char *tab_id, const char *widget_id,
                  const char *label) {
    return add_widget(ctx, tab_id, widget_id, label, AP_WIDGET_BUTTON, 0.0, 0.0, 0.0);
}

int ap_add_label(APContext *ctx, const char *tab_id, const char *widget_id,
                 const char *text) {
    return add_widget(ctx, tab_id, widget_id, text, AP_WIDGET_LABEL, 0.0, 0.0, 0.0);
}

int ap_get_toggle(APContext *ctx, const char *widget_id) {
    if (!valid(ctx)) return AP_ERR_INVALID_ARG;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    Widget *w = find_widget(ctx, widget_id);
    if (!w) return AP_ERR_NOT_FOUND;
    if (w->kind != AP_WIDGET_TOGGLE) return AP_ERR_WRONG_KIND;
    return w->value != 0.0 ? 1 : 0;
}

int ap_set_toggle(APContext *ctx, const char *widget_id, int value) {
    if (!valid(ctx)) return AP_ERR_INVALID_ARG;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    Widget *w = find_widget(ctx, widget_id);
    if (!w) return AP_ERR_NOT_FOUND;
    if (w->kind != AP_WIDGET_TOGGLE) return AP_ERR_WRONG_KIND;
    w->value = value ? 1.0 : 0.0;
    return AP_OK;
}

double ap_get_slider(APContext *ctx, const char *widget_id) {
    if (!valid(ctx)) return 0.0;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    Widget *w = find_widget(ctx, widget_id);
    if (!w || w->kind != AP_WIDGET_SLIDER) return 0.0;
    return w->value;
}

int ap_set_slider(APContext *ctx, const char *widget_id, double value) {
    if (!valid(ctx)) return AP_ERR_INVALID_ARG;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    Widget *w = find_widget(ctx, widget_id);
    if (!w) return AP_ERR_NOT_FOUND;
    if (w->kind != AP_WIDGET_SLIDER) return AP_ERR_WRONG_KIND;
    if (value < w->min_value || value > w->max_value) return AP_ERR_OUT_OF_RANGE;
    w->value = value;
    return AP_OK;
}

void ap_set_callback(APContext *ctx, APEventCallback cb, void *user_data) {
    if (!valid(ctx)) return;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    ctx->callback  = cb;
    ctx->user_data = user_data;
}

int ap_dispatch_event(APContext *ctx, const char *tab_id,
                      const char *widget_id, APEventKind kind, double value) {
    if (!valid(ctx) || !widget_id) return AP_ERR_INVALID_ARG;

    APEventCallback cb = nullptr;
    void *user = nullptr;
    std::string tab_copy;
    {
        std::lock_guard<std::mutex> lock(ctx->mutex);
        Widget *w = find_widget(ctx, widget_id);
        if (!w) return AP_ERR_NOT_FOUND;
        if (!w->enabled) return AP_ERR_INVALID_ARG;

        switch (kind) {
            case AP_EVENT_CLICK:
                if (w->kind != AP_WIDGET_BUTTON && w->kind != AP_WIDGET_TOGGLE)
                    return AP_ERR_WRONG_KIND;
                if (w->kind == AP_WIDGET_TOGGLE) w->value = (w->value != 0.0) ? 0.0 : 1.0;
                value = w->value;
                break;
            case AP_EVENT_VALUE_CHANGED:
                if (w->kind != AP_WIDGET_SLIDER) return AP_ERR_WRONG_KIND;
                if (value < w->min_value || value > w->max_value)
                    return AP_ERR_OUT_OF_RANGE;
                w->value = value;
                break;
            default:
                return AP_ERR_INVALID_ARG;
        }

        cb       = ctx->callback;
        user     = ctx->user_data;
        tab_copy = tab_id ? tab_id : w->tab_id;
    }

    if (cb) cb(user, tab_copy.c_str(), widget_id, kind, value);
    return AP_OK;
}

int ap_widget_count(APContext *ctx) {
    if (!valid(ctx)) return 0;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    return static_cast<int>(ctx->widgets.size());
}

int ap_widget_at(APContext *ctx, int index, APWidgetInfo *out) {
    if (!valid(ctx) || !out || index < 0) return AP_ERR_INVALID_ARG;
    std::lock_guard<std::mutex> lock(ctx->mutex);
    if (index >= static_cast<int>(ctx->widgets.size())) return AP_ERR_NOT_FOUND;
    const Widget &w = ctx->widgets[static_cast<size_t>(index)];
    out->tab_id    = w.tab_id.c_str();
    out->id        = w.id.c_str();
    out->kind      = w.kind;
    out->label     = w.label.c_str();
    out->min_value = w.min_value;
    out->max_value = w.max_value;
    out->value     = w.value;
    out->enabled   = w.enabled ? 1 : 0;
    return AP_OK;
}

const char *ap_serialize(APContext *ctx) {
    if (!valid(ctx)) return nullptr;
    std::string json;
    {
        std::lock_guard<std::mutex> lock(ctx->mutex);
        std::ostringstream o;
        o << "{\"title\":\"" << escape_json(ctx->title)
          << "\",\"version\":\"" << escape_json(ctx->version)
          << "\",\"author\":\"" << escape_json(ctx->author)
          << "\",\"api\":" << AP_API_VERSION
          << ",\"tabs\":[";
        for (size_t i = 0; i < ctx->tabs.size(); ++i) {
            if (i) o << ',';
            o << "{\"id\":\"" << escape_json(ctx->tabs[i].id)
              << "\",\"title\":\"" << escape_json(ctx->tabs[i].title) << "\"}";
        }
        o << "],\"widgets\":[";
        for (size_t i = 0; i < ctx->widgets.size(); ++i) {
            const Widget &w = ctx->widgets[i];
            if (i) o << ',';
            o << "{\"tab\":\"" << escape_json(w.tab_id)
              << "\",\"id\":\"" << escape_json(w.id)
              << "\",\"kind\":\"" << kind_name(w.kind)
              << "\",\"label\":\"" << escape_json(w.label)
              << "\",\"min\":" << w.min_value
              << ",\"max\":" << w.max_value
              << ",\"value\":" << w.value
              << ",\"enabled\":" << (w.enabled ? "true" : "false") << '}';
        }
        o << "]}";
        json = o.str();
    }

    char *buf = static_cast<char *>(std::malloc(json.size() + 1));
    if (!buf) return nullptr;
    std::memcpy(buf, json.c_str(), json.size() + 1);
    return buf;
}

void ap_free_string(const char *str) { std::free(const_cast<char *>(str)); }

} // extern "C"

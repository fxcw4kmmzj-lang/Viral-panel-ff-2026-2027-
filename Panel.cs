// AdvancedPanel — managed wrapper.
// Copyright (c) 2026 Sotos. All rights reserved. See LICENSE.

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace AdvancedPanel
{
    public enum PanelTheme { Dark = 0, Light = 1 }

    public enum WidgetKind { Toggle = 0, Slider = 1, Button = 2, Label = 3 }

    public enum EventKind { Click = 1, ValueChanged = 2 }

    public sealed class PanelWidget
    {
        public string TabId { get; init; } = "";
        public string Id { get; init; } = "";
        public WidgetKind Kind { get; init; }
        public string Label { get; init; } = "";
        public double Min { get; init; }
        public double Max { get; init; }
        public double Value { get; init; }
        public bool Enabled { get; init; }
    }

    public sealed class PanelEventArgs : EventArgs
    {
        public string TabId { get; }
        public string WidgetId { get; }
        public EventKind Kind { get; }
        public double Value { get; }

        public PanelEventArgs(string tabId, string widgetId, EventKind kind, double value)
        {
            TabId = tabId;
            WidgetId = widgetId;
            Kind = kind;
            Value = value;
        }
    }

    /// <summary>
    /// Managed handle over the AdvancedPanel native dylib.
    /// Safe to use from multiple threads; the native side is mutex-guarded.
    /// </summary>
    public sealed class Panel : IDisposable
    {
        private IntPtr _ctx;
        private NativeMethods.EventCallback? _callback; // kept alive for native code
        private bool _disposed;

        public string Title { get; }
        public string Version { get; }
        public string Author { get; }

        public event EventHandler<PanelEventArgs>? WidgetChanged;

        public static string NativeVersion =>
            Marshal.PtrToStringAnsi(NativeMethods.ap_version()) ?? "";
        public static int ApiVersion => NativeMethods.ap_api_version();

        public Panel(string title, string version, string author,
                             int width = 1280, int height = 720,
                             PanelTheme theme = PanelTheme.Dark)
        {
            Title = title;
            Version = version;
            Author = author;

            var config = new NativeMethods.NativeConfig
            {
                Title = Marshal.StringToHGlobalAnsi(title),
                Version = Marshal.StringToHGlobalAnsi(version),
                Author = Marshal.StringToHGlobalAnsi(author),
                Width = width,
                Height = height,
                Theme = (int)theme,
                Flags = 0
            };

            try
            {
                _ctx = NativeMethods.ap_create(ref config);
            }
            finally
            {
                Marshal.FreeHGlobal(config.Title);
                Marshal.FreeHGlobal(config.Version);
                Marshal.FreeHGlobal(config.Author);
            }

            if (_ctx == IntPtr.Zero)
                throw new InvalidOperationException("ap_create returned a null context.");

            _callback = OnNativeEvent;
            NativeMethods.ap_set_callback(_ctx, _callback, IntPtr.Zero);
        }

        private void OnNativeEvent(IntPtr userData, IntPtr tabId, IntPtr widgetId,
                                   int kind, double value)
        {
            WidgetChanged?.Invoke(this, new PanelEventArgs(
                Marshal.PtrToStringAnsi(tabId) ?? "",
                Marshal.PtrToStringAnsi(widgetId) ?? "",
                (EventKind)kind,
                value));
        }

        private void ThrowIfDisposed()
        {
            if (_disposed) throw new ObjectDisposedException(nameof(Panel));
        }

        public int TabCount { get { ThrowIfDisposed(); return NativeMethods.ap_tab_count(_ctx); } }

        public bool AddTab(string tabId, string title)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_add_tab(_ctx, tabId, title) == 0;
        }

        public bool RemoveTab(string tabId)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_remove_tab(_ctx, tabId) == 0;
        }

        public bool AddToggle(string tabId, string widgetId, string label, bool initial = false)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_add_toggle(_ctx, tabId, widgetId, label, initial ? 1 : 0) == 0;
        }

        public bool AddSlider(string tabId, string widgetId, string label,
                              double min, double max, double initial)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_add_slider(_ctx, tabId, widgetId, label, min, max, initial) == 0;
        }

        public bool AddButton(string tabId, string widgetId, string label)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_add_button(_ctx, tabId, widgetId, label) == 0;
        }

        public bool AddLabel(string tabId, string widgetId, string text)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_add_label(_ctx, tabId, widgetId, text) == 0;
        }

        public bool GetToggle(string widgetId)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_get_toggle(_ctx, widgetId) == 1;
        }

        public bool SetToggle(string widgetId, bool value)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_set_toggle(_ctx, widgetId, value ? 1 : 0) == 0;
        }

        public double GetSlider(string widgetId)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_get_slider(_ctx, widgetId);
        }

        public bool SetSlider(string widgetId, double value)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_set_slider(_ctx, widgetId, value) == 0;
        }

        public bool Click(string tabId, string widgetId)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_dispatch_event(_ctx, tabId, widgetId, (int)EventKind.Click, 0.0) == 0;
        }

        public bool SetSliderFromUI(string tabId, string widgetId, double value)
        {
            ThrowIfDisposed();
            return NativeMethods.ap_dispatch_event(_ctx, tabId, widgetId,
                (int)EventKind.ValueChanged, value) == 0;
        }

        public IReadOnlyList<PanelWidget> Widgets
        {
            get
            {
                ThrowIfDisposed();
                var list = new List<PanelWidget>();
                int count = NativeMethods.ap_widget_count(_ctx);
                for (int i = 0; i < count; i++)
                {
                    if (NativeMethods.ap_widget_at(_ctx, i, out var info) != 0) continue;
                    list.Add(new PanelWidget
                    {
                        TabId = Marshal.PtrToStringAnsi(info.TabId) ?? "",
                        Id = Marshal.PtrToStringAnsi(info.Id) ?? "",
                        Kind = (WidgetKind)info.Kind,
                        Label = Marshal.PtrToStringAnsi(info.Label) ?? "",
                        Min = info.MinValue,
                        Max = info.MaxValue,
                        Value = info.Value,
                        Enabled = info.Enabled != 0
                    });
                }
                return list;
            }
        }

        public string ToJson()
        {
            ThrowIfDisposed();
            IntPtr ptr = NativeMethods.ap_serialize(_ctx);
            if (ptr == IntPtr.Zero) return "{}";
            try { return Marshal.PtrToStringAnsi(ptr) ?? "{}"; }
            finally { NativeMethods.ap_free_string(ptr); }
        }

        public void Dispose()
        {
            if (_disposed) return;
            _disposed = true;
            if (_ctx != IntPtr.Zero)
            {
                NativeMethods.ap_destroy(_ctx);
                _ctx = IntPtr.Zero;
            }
            _callback = null;
        }

        ~Panel() { Dispose(); }
    }
}

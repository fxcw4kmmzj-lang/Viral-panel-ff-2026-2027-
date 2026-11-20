// AdvancedPanel — P/Invoke layer.
// Copyright (c) 2026 Sotos. All rights reserved. See LICENSE.

using System;
using System.Runtime.InteropServices;

namespace AdvancedPanel
{
    internal static class NativeMethods
    {
        // "advanced_panel" resolves to libadvanced_panel.dylib on Apple
        // platforms, advanced_panel.dll on Windows, libadvanced_panel.so on Linux.
        private const string Library = "advanced_panel";

        [StructLayout(LayoutKind.Sequential)]
        internal struct NativeConfig
        {
            public IntPtr Title;
            public IntPtr Version;
            public IntPtr Author;
            public int Width;
            public int Height;
            public int Theme;
            public uint Flags;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct NativeWidgetInfo
        {
            public IntPtr TabId;
            public IntPtr Id;
            public int Kind;
            public IntPtr Label;
            public double MinValue;
            public double MaxValue;
            public double Value;
            public int Enabled;
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void EventCallback(IntPtr userData, IntPtr tabId,
                                             IntPtr widgetId, int kind, double value);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr ap_version();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_api_version();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr ap_build_info();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr ap_create(ref NativeConfig config);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ap_destroy(IntPtr ctx);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_add_tab(IntPtr ctx, string tabId, string title);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_remove_tab(IntPtr ctx, string tabId);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_tab_count(IntPtr ctx);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_add_toggle(IntPtr ctx, string tabId, string widgetId,
                                                 string label, int initialValue);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_add_slider(IntPtr ctx, string tabId, string widgetId,
                                                 string label, double min, double max,
                                                 double initialValue);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_add_button(IntPtr ctx, string tabId, string widgetId,
                                                 string label);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_add_label(IntPtr ctx, string tabId, string widgetId,
                                                string text);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_get_toggle(IntPtr ctx, string widgetId);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_set_toggle(IntPtr ctx, string widgetId, int value);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern double ap_get_slider(IntPtr ctx, string widgetId);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_set_slider(IntPtr ctx, string widgetId, double value);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ap_set_callback(IntPtr ctx, EventCallback callback,
                                                    IntPtr userData);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_dispatch_event(IntPtr ctx, string tabId, string widgetId,
                                                     int kind, double value);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_widget_count(IntPtr ctx);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ap_widget_at(IntPtr ctx, int index,
                                                out NativeWidgetInfo info);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr ap_serialize(IntPtr ctx);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ap_free_string(IntPtr str);
    }
}

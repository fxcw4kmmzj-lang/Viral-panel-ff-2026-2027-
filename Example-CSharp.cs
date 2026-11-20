// AdvancedPanel — C# host example.
// Copyright (c) 2026 Sotos. All rights reserved. See LICENSE.

using System;
using AdvancedPanel;

using var panel = new Panel("Advanced Panel", "1.0.0", "Sotos");

panel.WidgetChanged += (_, e) =>
    Console.WriteLine($"[event] tab={e.TabId} widget={e.WidgetId} kind={e.Kind} value={e.Value:F3}");

panel.AddTab("graphics", "Graphics");
panel.AddTab("audio", "Audio");

panel.AddToggle("graphics", "vsync", "V-Sync", initial: true);
panel.AddSlider("graphics", "render_scale", "Render Scale", 0.5, 2.0, 1.0);
panel.AddSlider("audio", "master", "Master Volume", 0.0, 1.0, 0.8);
panel.AddButton("audio", "reset", "Reset to Defaults");

panel.Click("graphics", "vsync");
panel.SetSliderFromUI("audio", "master", 0.35);

Console.WriteLine(panel.ToJson());

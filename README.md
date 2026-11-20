# AdvancedPanel

An advanced in-game settings / dev panel, shipped as a native **dylib**. One C++
core, three ways to call it: **C ABI**, **Objective-C** (macOS + iOS) and **C#**.

Copyright (c) 2026 Sotos. All rights reserved. Proprietary and confidential —
see `LICENSE`. Part of a commercial game project; do not redistribute without
permission.

## Layout — everything is in this one folder

| File | What it is |
| --- | --- |
| `advanced_panel.h` | The stable C ABI — the contract everything binds to |
| `panel.cpp` | Core: tabs, widgets, state, events, JSON snapshot |
| `APAdvancedPanel.h` / `.mm` | Objective-C wrapper (Foundation, ARC-safe) |
| `NativeMethods.cs` / `Panel.cs` | C# P/Invoke layer + `IDisposable` wrapper |
| `AdvancedPanel.csproj` | .NET project for the binding |
| `Example-CPP.cpp` / `Example-ObjC.m` / `Example-CSharp.cs` | One runnable example each |
| `CMakeLists.txt` | Builds the dylib, the ObjC wrapper, the C++ example |
| `build-macos.sh` / `build-ios-xcframework.sh` | Local build helpers |
| `LICENSE` | Your copyright notice |

The only folder is `.github/workflows/`, because GitHub requires that exact
path and there is no way around it.

## The ABI in one paragraph

`ap_create()` gives you an opaque `APContext*`. Add tabs, then add widgets
(toggles, sliders, buttons, labels) into those tabs. The host UI calls
`ap_dispatch_event()` when the player touches something; the core updates its
state and fires your callback. `ap_serialize()` returns a JSON snapshot of the
whole panel — everything you need to save and restore settings.

Every function returns an `APResult` code (`AP_OK` is `0`), so nothing is
silently swallowed. The context is mutex-guarded: safe to call from a render
thread and a UI thread at once.

## Build it

### macOS — universal dylib (arm64 + x86_64)

```bash
brew install cmake
./build-macos.sh
lipo -info dist/macos/lib/libadvanced_panel.dylib   # → arm64 x86_64
```

### Manual CMake

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/advanced_panel_example
```

### iOS — .xcframework

```bash
./build-ios-xcframework.sh
# → dist/ios/advanced_panel.xcframework (device arm64 + simulator arm64/x86_64)
```

### C#

```bash
dotnet build AdvancedPanel.csproj -c Release
dotnet pack  AdvancedPanel.csproj -c Release -o dist/nuget
```

The managed side looks for `advanced_panel` — `libadvanced_panel.dylib` on
Apple platforms. On iOS, link the static slice into your app and the symbols
are already there.

## The GitHub workflow

`.github/workflows/build.yml` runs on every push to `main`, every pull request,
and every `v*` tag. Four jobs:

| Job | What it produces |
| --- | --- |
| `macos-dylib` | Universal `.dylib`, `lipo`-verified, with `nm` checks that the ABI symbols are actually exported |
| `ios-xcframework` | Device + simulator static slices assembled into an `.xcframework` |
| `csharp-binding` | NuGet package from the C# binding |
| `release` | On a `v*` tag only: attaches everything to a GitHub Release |

Artifacts land on every run. Push a tag and you also get a release:

```bash
git tag v1.0.0 && git push origin v1.0.0
```

Two deliberate guards, because these are the usual silent failures:

- **`lipo -info | grep -q "arm64 x86_64"`** — a build that quietly produces a
  single-arch dylib fails the job instead of shipping a broken binary.
- **`nm -gU ... | grep -q "_ap_create"`** — hidden visibility can strip the whole
  ABI if the export macro is wrong; this catches it at build time.

The `libtool -static` step merges core + ObjC wrapper into one archive per
slice, since `xcodebuild -create-xcframework` wants one library and one header
directory per platform.

## Before you publish — three things

1. **Rename the project.** Replace `AdvancedPanel` / `advanced_panel` / `AP`
   across the tree:

   ```bash
   grep -rl 'advanced_panel' . | xargs sed -i '' 's/advanced_panel/yourlib/g'
   grep -rl 'AdvancedPanel'  . | xargs sed -i '' 's/AdvancedPanel/YourLib/g'
   ```

   Then rename the files themselves to match. Keep the `AP` prefix search tight
   or it will also hit the `AP_` enums — those are meant to change too, so a
   blanket replace is fine as long as you check the diff.

2. **Set your copyright.** It is already `Sotos` in `LICENSE` and in the header
   comment of every source file. Change it to your registered legal entity if
   the game is held by a company.

3. **Set bundle identifier and signing** in your Xcode project for iOS — the
   workflow builds unsigned slices only, by design.

## Wiring the panel to your renderer

The core is deliberately renderer-agnostic: it owns state and layout data, not
pixels. To put it on screen:

- **Dear ImGui** — iterate `ap_widget_at()` once per frame, emit
  `ImGui::Checkbox` / `ImGui::SliderScalar` / `ImGui::Button`, and call
  `ap_dispatch_event()` when a widget reports a change. Shortest route, works
  on macOS, iOS and Windows from one code path.
- **Unity** — use the C# binding and build the panel as a UGUI prefab. The
  binding targets `netstandard2.1`, so it drops into Unity 2021+.
- **Native AppKit / UIKit** — use `APAdvancedPanel` and back the tabs with
  `NSTabView` / `UITabBarController`.

Tell me which renderer your game uses and I'll write that binding against this
exact ABI without touching the core.

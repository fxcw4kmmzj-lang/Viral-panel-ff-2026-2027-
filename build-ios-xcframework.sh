#!/usr/bin/env bash
# Build an iOS .xcframework containing device + simulator slices.
# Copyright (c) 2026 Sotos. All rights reserved. See LICENSE.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT="${ROOT}/build/ios"
DIST="${ROOT}/dist/ios"
rm -rf "${OUT}" "${DIST}"
mkdir -p "${DIST}"

build_slice () {
    local name="$1" sysroot="$2" archs="$3"
    cmake -S "${ROOT}" -B "${OUT}/${name}" \
        -DCMAKE_SYSTEM_NAME=iOS \
        -DCMAKE_OSX_SYSROOT="${sysroot}" \
        -DCMAKE_OSX_ARCHITECTURES="${archs}" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="13.0" \
        -DCMAKE_BUILD_TYPE=Release \
        -DAP_BUILD_EXAMPLES=OFF
    cmake --build "${OUT}/${name}" --config Release -j "$(sysctl -n hw.ncpu)"
    mkdir -p "${OUT}/lib-${name}/lib" "${OUT}/lib-${name}/include/AdvancedPanel"
    libtool -static -o "${OUT}/lib-${name}/lib/libadvanced_panel.a" \
        "${OUT}/${name}"/CMakeFiles/advanced_panel.dir/panel.cpp.o \
        "${OUT}/${name}"/CMakeFiles/AdvancedPanelObjC.dir/APAdvancedPanel.mm.o
    cp "${ROOT}/advanced_panel.h" "${ROOT}/APAdvancedPanel.h" \
       "${OUT}/lib-${name}/include/AdvancedPanel/"
}

build_slice device    iphoneos        "arm64"
build_slice simulator iphonesimulator "arm64;x86_64"

xcodebuild -create-xcframework \
    -library "${OUT}/lib-device/lib/libadvanced_panel.a" \
    -headers "${OUT}/lib-device/include" \
    -library "${OUT}/lib-simulator/lib/libadvanced_panel.a" \
    -headers "${OUT}/lib-simulator/include" \
    -output "${DIST}/advanced_panel.xcframework"

echo "Created: ${DIST}/advanced_panel.xcframework"

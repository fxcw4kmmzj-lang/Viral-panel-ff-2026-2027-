#!/usr/bin/env bash
# Build a universal macOS dylib (arm64 + x86_64).
# Copyright (c) 2026 Sotos. All rights reserved. See LICENSE.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="${ROOT}/build/macos"
DIST="${ROOT}/dist/macos"

cmake -S "${ROOT}" -B "${BUILD}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="11.0" \
    -DAP_BUILD_EXAMPLES=OFF

cmake --build "${BUILD}" --config Release -j "$(sysctl -n hw.ncpu)"

mkdir -p "${DIST}/lib" "${DIST}/include"
cp "${BUILD}/libadvanced_panel.dylib" "${DIST}/lib/"
cp "${ROOT}/advanced_panel.h" "${ROOT}/APAdvancedPanel.h" "${DIST}/include/"

echo "Universal dylib:"
lipo -info "${DIST}/lib/libadvanced_panel.dylib"

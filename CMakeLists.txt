cmake_minimum_required(VERSION 3.20)

project(AdvancedPanel
    VERSION 1.0.0
    DESCRIPTION "Advanced in-game panel GUI"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(AP_BUILD_EXAMPLES "Build the C++ example" ON)
option(AP_BUILD_OBJC     "Build the Objective-C wrapper (Apple only)" ON)

# Flat layout: every source file lives next to this CMakeLists.
set(AP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ---------------------------------------------------------------- core dylib
add_library(advanced_panel SHARED panel.cpp)
add_library(AdvancedPanel::Core ALIAS advanced_panel)

target_include_directories(advanced_panel
    PUBLIC
        $<BUILD_INTERFACE:${AP_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>)

target_compile_definitions(advanced_panel PRIVATE AP_BUILDING_LIBRARY=1)

set_target_properties(advanced_panel PROPERTIES
    OUTPUT_NAME "advanced_panel"
    VERSION     ${PROJECT_VERSION}
    SOVERSION   1
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)

if(APPLE)
    set_target_properties(advanced_panel PROPERTIES
        INSTALL_RPATH "@loader_path"
        SUFFIX ".dylib")
endif()

if(MSVC)
    target_compile_options(advanced_panel PRIVATE /W4)
else()
    target_compile_options(advanced_panel PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ------------------------------------------------------- Objective-C wrapper
if(APPLE AND AP_BUILD_OBJC)
    enable_language(OBJCXX)

    add_library(AdvancedPanelObjC STATIC APAdvancedPanel.mm)
    target_link_libraries(AdvancedPanelObjC PUBLIC advanced_panel "-framework Foundation")
    target_include_directories(AdvancedPanelObjC PUBLIC ${AP_INCLUDE_DIR})
endif()

# ------------------------------------------------------------------- example
if(AP_BUILD_EXAMPLES)
    add_executable(advanced_panel_example Example-CPP.cpp)
    target_link_libraries(advanced_panel_example PRIVATE advanced_panel)
endif()

# ------------------------------------------------------------------- install
include(GNUInstallDirs)

install(TARGETS advanced_panel
    EXPORT  AdvancedPanelTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES advanced_panel.h APAdvancedPanel.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT AdvancedPanelTargets
    FILE      AdvancedPanelTargets.cmake
    NAMESPACE AdvancedPanel::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AdvancedPanel)

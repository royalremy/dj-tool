cmake_minimum_required(VERSION 3.15)
project(DJEditLab VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# JUCE as subdirectory (git submodule in third_party/JUCE)
add_subdirectory(third_party/JUCE)

# App definition
juce_add_gui_app(DJEditLab
    PRODUCT_NAME "DJ Edit Lab"
    VERSION "1.0.0"
    BUNDLE_ID "com.royalremy.djtool"
    MICROPHONE_PERMISSION_ENABLED TRUE
    NEEDS_CURL FALSE
    NEEDS_WEB_BROWSER FALSE
)

# JUCE header generation
juce_generate_juce_header(DJEditLab)

# Source files
target_sources(DJEditLab PRIVATE
    app/main.cpp
    engine/AudioEngine.cpp
    engine/MasterClock.cpp
    engine/EventScheduler.cpp
    dsp/EditSystem.cpp
    ui/MainComponent.cpp
    ui/TimelineComponent.cpp
)

# Include directories
target_include_directories(DJEditLab PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler flags
target_compile_options(DJEditLab PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Release>:-O2>
)

# JUCE modules
target_link_libraries(DJEditLab PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# macOS: Sign with debug entitlements so LLDB can attach (Debug only)
if(APPLE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET DJEditLab POST_BUILD
        COMMAND codesign --force --sign - --entitlements
            "${CMAKE_SOURCE_DIR}/resources/debug.entitlements"
            "$<TARGET_BUNDLE_DIR:DJEditLab>"
        COMMENT "Signing with debug entitlements (allows LLDB attach)"
    )
endif()

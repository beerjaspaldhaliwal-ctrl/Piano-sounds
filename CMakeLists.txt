cmake_minimum_required(VERSION 3.22)

project(Keyverse VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(KEYVERSE_BUILD_PLUGIN "Build the JUCE plug-in (VST3 / AU / LV2 / Standalone)" ON)
option(KEYVERSE_BUILD_TOOLS  "Build the offline renderer and preset tests" ON)
option(KEYVERSE_LTO          "Link-time optimisation for release plug-in builds" ON)
set(KEYVERSE_JUCE_TAG "9.0.4" CACHE STRING "JUCE git tag to fetch")

#-------------------------------------------------------------------------------
# Sound engine: plain C++17, no dependencies (shared by plug-in and tools)
file(GLOB KEYVERSE_BANKS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Source/Engine/Banks/*.cpp)
add_library(keyverse_engine STATIC
    Source/Engine/PresetLibrary.cpp
    ${KEYVERSE_BANKS})
target_include_directories(keyverse_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/Engine)
target_compile_features(keyverse_engine PUBLIC cxx_std_17)
set_target_properties(keyverse_engine PROPERTIES POSITION_INDEPENDENT_CODE ON)
if (MSVC)
    target_compile_options(keyverse_engine PUBLIC /fp:fast /bigobj)
else()
    target_compile_options(keyverse_engine PUBLIC -fopenmp-simd -fno-math-errno)
endif()

#-------------------------------------------------------------------------------
if (KEYVERSE_BUILD_TOOLS)
    add_executable(kvrender tools/kvrender.cpp)
    target_link_libraries(kvrender PRIVATE keyverse_engine)
    enable_testing()
    add_test(NAME factory_presets COMMAND kvrender --quiet)
endif()

#-------------------------------------------------------------------------------
if (KEYVERSE_BUILD_PLUGIN)
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        ${KEYVERSE_JUCE_TAG}
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)

    set(KEYVERSE_FORMATS VST3 Standalone)
    if (APPLE)
        list(APPEND KEYVERSE_FORMATS AU)
    endif()
    if (UNIX AND NOT APPLE)
        list(APPEND KEYVERSE_FORMATS LV2)
    endif()

    juce_add_plugin(Keyverse
        COMPANY_NAME                "Keyverse Audio"
        COMPANY_WEBSITE             "https://github.com/beerjaspaldhaliwal-ctrl/Piano-sounds"
        BUNDLE_ID                   com.keyverse.keyverse
        PLUGIN_MANUFACTURER_CODE    Kyvs
        PLUGIN_CODE                 Kyv1
        FORMATS                     ${KEYVERSE_FORMATS}
        PRODUCT_NAME                "Keyverse"
        DESCRIPTION                 "Pianos, keys, synths and guitars instrument"
        IS_SYNTH                    TRUE
        NEEDS_MIDI_INPUT            TRUE
        NEEDS_MIDI_OUTPUT           FALSE
        IS_MIDI_EFFECT              FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD     FALSE
        VST3_CATEGORIES             Instrument Synth
        AU_MAIN_TYPE                kAudioUnitType_MusicDevice
        LV2URI                      "https://github.com/beerjaspaldhaliwal-ctrl/Piano-sounds")

    target_sources(Keyverse PRIVATE
        Source/Plugin/PluginProcessor.cpp
        Source/Plugin/PluginEditor.cpp)

    target_compile_definitions(Keyverse PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_XINPUT=0)

    target_link_libraries(Keyverse
        PRIVATE
            keyverse_engine
            juce::juce_audio_utils
            juce::juce_audio_processors
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
    if (KEYVERSE_LTO)
        target_link_libraries(Keyverse PUBLIC juce::juce_recommended_lto_flags)
    endif()
endif()

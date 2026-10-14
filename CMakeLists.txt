cmake_minimum_required(VERSION 3.22)

project(BKPOptoGlow VERSION 1.0.0 LANGUAGES C CXX)

# Static MSVC runtime: the plugin needs no Visual C++ Redistributable on the user PC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# JUCE: use a local checkout if JUCE_LOCAL_PATH is given, otherwise download it
# ---------------------------------------------------------------------------
if(DEFINED JUCE_LOCAL_PATH)
    add_subdirectory(${JUCE_LOCAL_PATH} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        9.0.4
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

option(BKP_BUILD_TESTS "Build the offline test runner" OFF)

# moduleinfo.json generation runs the plugin, which is impossible when cross-compiling
if(CMAKE_CROSSCOMPILING)
    set(BKP_VST3_MANIFEST FALSE)
else()
    set(BKP_VST3_MANIFEST TRUE)
endif()

juce_add_plugin(BKPOptoGlow
    COMPANY_NAME                "ROHIT JI"
    BUNDLE_ID                   com.rohitji.optoglow
    PLUGIN_MANUFACTURER_CODE    Rhji
    PLUGIN_CODE                 Bkog
    PRODUCT_NAME                "Opto Glow"
    DESCRIPTION                 "Vintage opto compressor with tone and saturation"
    FORMATS                     VST3
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             Fx Dynamics
    VST3_AUTO_MANIFEST          ${BKP_VST3_MANIFEST})

set(BKP_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_sources(BKPOptoGlow PRIVATE ${BKP_SOURCES})

set(BKP_DEFINITIONS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

if(WIN32)
    list(APPEND BKP_DEFINITIONS WINVER=0x0A00 _WIN32_WINNT=0x0A00 JUCE_USE_WINDOWS_MEDIA_FORMAT=0)
endif()

if(UNIX AND NOT APPLE)
    # Keep Linux test builds light on system dependencies
    list(APPEND BKP_DEFINITIONS JUCE_ALSA=0 JUCE_JACK=0 JUCE_USE_XINPUT=0
         JUCE_USE_XRANDR=0 JUCE_USE_XINERAMA=0 JUCE_USE_XCURSOR=0 JUCE_USE_XRENDER=0)
endif()

target_compile_definitions(BKPOptoGlow PUBLIC ${BKP_DEFINITIONS})

target_link_libraries(BKPOptoGlow
    PRIVATE
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# MinGW ignores MSVC's "#pragma comment(lib)", so list the Windows libraries
if(MINGW)
    target_link_libraries(BKPOptoGlow PRIVATE
        sapi d2d1 dxgi d3d11 dcomp shcore version imm32 dwmapi comctl32 dwrite
        winmm ws2_32 wininet shlwapi ole32 oleaut32 uuid rpcrt4 setupapi crypt32 dxguid dbghelp)
    target_link_options(BKPOptoGlow PRIVATE -static)
endif()

# ---------------------------------------------------------------------------
# Offline test runner (DSP checks + UI screenshot) - not needed for the plugin
# ---------------------------------------------------------------------------
if(BKP_BUILD_TESTS)
    juce_add_console_app(BKPTests PRODUCT_NAME "BKPTests")
    target_sources(BKPTests PRIVATE ${BKP_SOURCES} Tests/TestMain.cpp)
    target_compile_definitions(BKPTests PRIVATE ${BKP_DEFINITIONS} JucePlugin_Name="Opto Glow"
        JucePlugin_Build_VST3=0 BKP_TEST_BUILD=1 JUCE_MODAL_LOOPS_PERMITTED=1)
    target_include_directories(BKPTests PRIVATE Source)
    target_link_libraries(BKPTests PRIVATE
        juce::juce_audio_processors juce::juce_dsp juce::juce_gui_basics
        juce::juce_recommended_config_flags)
endif()

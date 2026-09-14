cmake_minimum_required(VERSION 3.22)

project(TinyDAW VERSION 0.0.1)

# Pulls JUCE's source down at configure time and makes its CMake
# functions (juce_add_gui_app, etc.) available to this file.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Declares the TinyDAW target as a JUCE GUI application and wires up
# all the platform-specific plumbing (Info.plist, .exe icon, etc.)
juce_add_gui_app(TinyDAW
    PRODUCT_NAME "Tiny DAW"
)

target_sources(TinyDAW
    PRIVATE
        Source/Main.cpp
)

target_compile_definitions(TinyDAW
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:TinyDAW,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:TinyDAW,JUCE_VERSION>"
)

target_link_libraries(TinyDAW
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

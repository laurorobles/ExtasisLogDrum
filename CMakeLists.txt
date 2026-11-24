cmake_minimum_required(VERSION 3.15)
project(ExtasisLogDrum VERSION 1.0.0 LANGUAGES C CXX)

option(EXTASIS_INSTALL_LOCAL "Copy built plugins to user Plug-Ins directory on macOS" ON)

# Fetch JUCE 8 (shallow clone for fast open-source builds)
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(juce)

# Embedded binary assets
juce_add_binary_data(ExtasisLogDrum_Data
    SOURCES
        assets/logo.png
)

# Configure the JUCE plugin
juce_add_plugin(ExtasisLogDrum
    COMPANY_NAME "Extasis Records"
    BUNDLE_ID "com.ExtasisRecords.ExtasisLogDrum"
    COMPANY_WEBSITE "https://github.com/laurorobles/ExtasisLogDrum"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Exta
    PLUGIN_CODE LgDr
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "ExtasisLogDrum"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.png"
)

juce_generate_juce_header(ExtasisLogDrum)

# Add source files
target_sources(ExtasisLogDrum PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/GUI/BeeperTriggerButton.cpp
    Source/GUI/BeeperDisplay.cpp
    Source/GUI/BeeperLookAndFeel.cpp
)

# Compile features and definitions
target_compile_features(ExtasisLogDrum PRIVATE cxx_std_20)
target_compile_definitions(ExtasisLogDrum PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(ExtasisLogDrum PRIVATE
    ExtasisLogDrum_Data
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Optional local installation on macOS (skipped automatically in CI)
if (APPLE AND EXTASIS_INSTALL_LOCAL AND NOT DEFINED ENV{CI})
    if (TARGET ExtasisLogDrum_VST3)
        add_custom_command(TARGET ExtasisLogDrum_VST3 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
            COMMAND ${CMAKE_COMMAND} -E remove_directory "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/ExtasisLogDrum.vst3"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "$<TARGET_BUNDLE_DIR:ExtasisLogDrum_VST3>"
                "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/ExtasisLogDrum.vst3"
            COMMENT "Installing VST3 to ~/Library/Audio/Plug-Ins/VST3/"
        )
    endif()

    if (TARGET ExtasisLogDrum_AU)
        add_custom_command(TARGET ExtasisLogDrum_AU POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/Library/Audio/Plug-Ins/Components"
            COMMAND ${CMAKE_COMMAND} -E remove_directory "$ENV{HOME}/Library/Audio/Plug-Ins/Components/ExtasisLogDrum.component"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "$<TARGET_BUNDLE_DIR:ExtasisLogDrum_AU>"
                "$ENV{HOME}/Library/Audio/Plug-Ins/Components/ExtasisLogDrum.component"
            COMMENT "Installing AU to ~/Library/Audio/Plug-Ins/Components/"
        )
    endif()
endif()

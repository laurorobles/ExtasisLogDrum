cmake_minimum_required(VERSION 3.15)
project(ExtasisLogDrum VERSION 1.0.0)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(juce)

# Configure the JUCE plugin
juce_add_binary_data(ExtasisLogDrum_Data
    SOURCES
        assets/logo.png
)

juce_add_plugin(ExtasisLogDrum
    COMPANY_NAME "ExtasisRecords"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE Exta
    PLUGIN_CODE LgDr
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "ExtasisLogDrum"
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

# Compile features and links
target_compile_features(ExtasisLogDrum PRIVATE cxx_std_20)
target_compile_definitions(ExtasisLogDrum PUBLIC JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(ExtasisLogDrum PRIVATE
    ExtasisLogDrum_Data
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Custom post-build step to guarantee installation to user plug-in directory
if (APPLE)
    if (TARGET ExtasisLogDrum_VST3)
        add_custom_command(TARGET ExtasisLogDrum_VST3 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/Library/Audio/Plug-Ins/VST3"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "$<TARGET_BUNDLE_DIR:ExtasisLogDrum_VST3>"
                "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/ExtasisLogDrum.vst3"
            COMMENT "Installing VST3 to ~/Library/Audio/Plug-Ins/VST3/"
        )
    endif()

    if (TARGET ExtasisLogDrum_AU)
        add_custom_command(TARGET ExtasisLogDrum_AU POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/Library/Audio/Plug-Ins/Components"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "$<TARGET_BUNDLE_DIR:ExtasisLogDrum_AU>"
                "$ENV{HOME}/Library/Audio/Plug-Ins/Components/ExtasisLogDrum.component"
            COMMENT "Installing AU to ~/Library/Audio/Plug-Ins/Components/"
        )
    endif()
endif()

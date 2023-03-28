cmake_minimum_required(VERSION 3.15)
project(ElemEffectsPlugin VERSION 0.1.0)


set(TARGET_NAME FXP)
set(ASSETS_DIR js/dist/)

option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)

add_subdirectory(juce)
add_subdirectory(elementary/runtime)

juce_add_plugin(${TARGET_NAME}
  BUNDLE_ID "audio.elementary.fxp"
  COMPANY_NAME "Elementary Audio"
  COMPANY_WEBSITE "https://www.elementary.audio"
  COMPANY_EMAIL "nick@elementary.audio"
  PLUGIN_MANUFACTURER_CODE Elem       # A four-character manufacturer id with at least one upper-case character
  PLUGIN_CODE Fxpl                    # A unique four-character plugin id with at least one upper-case character
  COPY_PLUGIN_AFTER_BUILD TRUE        # Should the plugin be installed to a default location after building?
  APP_SANDBOX_ENABLED TRUE
  APP_SANDBOX_OPTIONS com.apple.security.network.client com.apple.security.files.user-selected.read-write
  FORMATS AU VST3 Standalone          # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
  PRODUCT_NAME ${TARGET_NAME})        # The name of the final executable, which can differ from the target name

juce_add_bundle_resources_directory(${TARGET_NAME}_VST3 ${ASSETS_DIR})
juce_add_bundle_resources_directory(${TARGET_NAME}_Standalone ${ASSETS_DIR})

if(TARGET ${TARGET_NAME}_AU)
  juce_add_bundle_resources_directory(${TARGET_NAME}_AU ${ASSETS_DIR})
endif()

target_sources(${TARGET_NAME}
  PRIVATE
  PluginProcessor.cpp)

target_include_directories(${TARGET_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/choc/javascript)

target_compile_features(${TARGET_NAME}
  PRIVATE
  cxx_std_17)

target_compile_definitions(${TARGET_NAME}
  PRIVATE
  JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_USE_CURL=0)

target_link_libraries(${TARGET_NAME}
  PRIVATE
  juce::juce_audio_basics
  juce::juce_audio_devices
  juce::juce_audio_plugin_client
  juce::juce_audio_processors
  juce::juce_audio_utils
  juce::juce_core
  juce::juce_data_structures
  juce::juce_dsp
  juce::juce_events
  juce::juce_graphics
  juce::juce_gui_basics
  juce::juce_gui_extra
  runtime)

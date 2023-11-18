cmake_minimum_required(VERSION 3.15)
project(Elem_SRVB VERSION 0.1.0)


set(TARGET_NAME SRVB)
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/../dist/)

option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(ELEM_DEV_LOCALHOST "Run against localhost for static assets" OFF)

add_subdirectory(juce)
add_subdirectory(elementary/runtime)

juce_add_plugin(${TARGET_NAME}
  BUNDLE_ID "audio.elementary.srvb"
  COMPANY_NAME "Elementary Audio"
  COMPANY_WEBSITE "https://www.elementary.audio"
  COMPANY_EMAIL "nick@elementary.audio"
  PLUGIN_MANUFACTURER_CODE Elem       # A four-character manufacturer id with at least one upper-case character
  PLUGIN_CODE Srvb                    # A unique four-character plugin id with at least one upper-case character
  COPY_PLUGIN_AFTER_BUILD FALSE       # We enable this manually below after adding a copy step
  APP_SANDBOX_ENABLED TRUE
  APP_SANDBOX_OPTIONS com.apple.security.network.client com.apple.security.files.user-selected.read-write
  FORMATS AU VST3                     # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
  PRODUCT_NAME ${TARGET_NAME})        # The name of the final executable, which can differ from the target name

# Copy static assets post build
if (NOT ELEM_DEV_LOCALHOST)
  get_target_property(ACTIVE_TARGETS ${TARGET_NAME} JUCE_ACTIVE_PLUGIN_TARGETS)
  foreach(ACTIVE_TARGET IN LISTS ACTIVE_TARGETS)
    message(STATUS "Adding resource copy step from ${ASSETS_DIR} for ${ACTIVE_TARGET}")

    get_target_property(ARTIFACT_FILE ${ACTIVE_TARGET} JUCE_PLUGIN_ARTEFACT_FILE)
    set(RESOURCE_DIR "${ARTIFACT_FILE}/Contents/Resources/")

    add_custom_command(TARGET ${ACTIVE_TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E rm -rf "${RESOURCE_DIR}/dist"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCE_DIR}/dist"
      COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}" "${RESOURCE_DIR}/dist"
      VERBATIM)
  endforeach()
endif()

# Enable copy step
if (NOT DEFINED ENV{CI})
  juce_enable_copy_plugin_step(${TARGET_NAME})
endif()

target_sources(${TARGET_NAME}
  PRIVATE
  PluginProcessor.cpp
  WebViewEditor.cpp)

target_include_directories(${TARGET_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/choc/gui
  ${CMAKE_CURRENT_SOURCE_DIR}/choc/javascript)

target_compile_features(${TARGET_NAME}
  PRIVATE
  cxx_std_17)

target_compile_definitions(${TARGET_NAME}
  PRIVATE
  ELEM_DEV_LOCALHOST=${ELEM_DEV_LOCALHOST}
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

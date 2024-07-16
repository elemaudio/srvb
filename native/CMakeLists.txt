cmake_minimum_required(VERSION 3.15)
project(NEL_SRVB VERSION 0.1.0)


set(TARGET_NAME NEL_SRVB)
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/../dist/)

option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
option(JUCE_BUILD_EXTRAS "Build JUCE Extras" OFF)
option(ELEM_DEV_LOCALHOST "Run against localhost for static assets" OFF)

add_subdirectory(juce)
add_subdirectory(elementary/runtime)

if (WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PC_ARCH "x64")
  else()
    set(PC_ARCH "x32")
  endif()
endif()

juce_add_plugin(${TARGET_NAME}
  BUNDLE_ID "audio.neverenginelabs.NEL_SRVB"
  COMPANY_NAME "NeverEngineLabs"
  COMPANY_WEBSITE "https://neverenginelabs.com"
  COMPANY_EMAIL "support@neverenginelabs.com"
  PLUGIN_MANUFACTURER_CODE NLVB       # A four-character manufacturer id with at least one upper-case character
  PLUGIN_CODE ShRv                    # A unique four-character plugin id with at least one upper-case character
  COPY_PLUGIN_AFTER_BUILD FALSE       # We enable this manually below after adding a copy step
  APP_SANDBOX_ENABLED TRUE 
  APP_SANDBOX_OPTIONS com.apple.security.network.client com.apple.security.files.user-selected.read-write
  FORMATS AU VST3                     # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
  PRODUCT_NAME "NELShifterMesh_${CMAKE_OSX_ARCHITECTURES}${PC_ARCH}_${BUILD_VERSION}")        # The name of the final executable, which can differ from the target name

  # Keyzy Client Library
  # Replace KeyzyClientLibraryX.X.X with your actual library version

set(KeyzyClientLibrary_VERSION "v143")

if(WIN32)
  if(MSVC_VERSION EQUAL 1910) # VS 2017
      set(VS_VERSION "VS2017")
  elseif(MSVC_VERSION EQUAL 1920) # VS 2019
      set(VS_VERSION "VS2019")
  elseif(MSVC_VERSION GREATER_EQUAL 1930) # VS 2022 or later
      set(VS_VERSION "VS2022")
  endif()

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(ARCH "64")
  else()
      set(ARCH "86")
  endif()

  # Set MD or MT based on your project requirements
  set(MD_MT "MD")

  set(DEBUG_OR_RELEASE "$<$<CONFIG:Debug>:D>$<$<CONFIG:Release>:R>")

  set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/KeyzyClientLibrary/${VS_VERSION}/${DEBUG_OR_RELEASE}${ARCH}${MD_MT}_x64_${KeyzyClientLibrary_VERSION})

  target_link_libraries(${TARGET_NAME} PRIVATE ${LIB_DIR}/Lib/KeyzyClient.lib)

elseif(APPLE)
  if(IOS)
      set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/KeyzyClientLibrary/XcodeiOS)
  else()
      # Choose the correct library based on your target architectures
      set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/KeyzyClientLibrary/XcodeAppleSiliconIntel64)
  endif()

  find_library(KEYZY_CLIENT_LIB KeyzyClient PATHS ${LIB_DIR}/Lib)
  if(NOT KEYZY_CLIENT_LIB)
      message(FATAL_ERROR "KeyzyClient library not found")
  endif()
  target_link_libraries(${TARGET_NAME} PRIVATE ${KEYZY_CLIENT_LIB})
endif()

target_include_directories(${TARGET_NAME} PRIVATE
  $<BUILD_INTERFACE:${LIB_DIR}/Include>)

# Copy static assets post build
if (NOT ELEM_DEV_LOCALHOST)
  get_target_property(ACTIVE_TARGETS ${TARGET_NAME} JUCE_ACTIVE_PLUGIN_TARGETS)
foreach(ACTIVE_TARGET IN LISTS ACTIVE_TARGETS)
  message(STATUS "Adding resource copy step from ${ASSETS_DIR} for ${ACTIVE_TARGET}")

  get_target_property(ARTIFACT_FILE ${ACTIVE_TARGET} JUCE_PLUGIN_ARTEFACT_FILE)
  set(RESOURCE_DIR "${ARTIFACT_FILE}/Contents/Resources/")

add_custom_command(TARGET ${ACTIVE_TARGET} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${RESOURCE_DIR}/dist"
  COMMAND ${CMAKE_COMMAND} -E echo "Removed directory: ${RESOURCE_DIR}/dist"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${RESOURCE_DIR}/dist"
  COMMAND ${CMAKE_COMMAND} -E echo "Created directory: ${RESOURCE_DIR}/dist"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}" "${RESOURCE_DIR}/dist"
  COMMAND ${CMAKE_COMMAND} -E echo "Copied directory: ${ASSETS_DIR} to ${RESOURCE_DIR}/dist"
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
  WebViewEditor.cpp
  )

target_include_directories(${TARGET_NAME}
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/choc/gui
  ${CMAKE_CURRENT_SOURCE_DIR}/choc/javascript
)

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
  # juce::juce_audio_devices
  # juce::juce_audio_plugin_client
  juce::juce_audio_processors
  juce::juce_audio_utils
  # juce::juce_core
  # juce::juce_data_structures
  # juce::juce_dsp
  # juce::juce_events
  # juce::juce_graphics
  # juce::juce_gui_basics
  juce::juce_gui_extra
  runtime
  )




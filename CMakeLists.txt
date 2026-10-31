cmake_minimum_required(VERSION 3.18)
project(arsonverb VERSION 0.5.0 LANGUAGES C CXX)
option(ARVERB_BUILD_TESTS "Build DSP unit tests and benchmarks" ON)
option(ARVERB_BUILD_SHARED "Build C ABI shared library for C#/other hosts" OFF)
option(ARVERB_BUILD_PLUGIN "Build JUCE 9 VST3 + standalone plugin" OFF)
set(ARVERB_JUCE_SOURCE_DIR "" CACHE PATH "Path to JUCE 9.0.4 source (otherwise fetch pinned tag)")

add_library(arverb_core STATIC src/Reverb.cpp src/Presets.cpp src/arverb_c.cpp)
# Product version is defined ONCE in CMake; both C ABI and plugin use it.
target_compile_definitions(arverb_core PRIVATE ARVERB_BUILD_VERSION="${PROJECT_VERSION}")
target_compile_features(arverb_core PUBLIC cxx_std_17)
target_include_directories(arverb_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
set_target_properties(arverb_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(arverb_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(arverb_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(ARVERB_BUILD_SHARED)
    add_library(arverb_shared SHARED src/Reverb.cpp src/Presets.cpp src/arverb_c.cpp)
    target_compile_definitions(arverb_shared PRIVATE ARVERB_BUILD_VERSION="${PROJECT_VERSION}")
    target_compile_features(arverb_shared PUBLIC cxx_std_17)
    target_include_directories(arverb_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_definitions(arverb_shared PRIVATE ARVERB_BUILD_SHARED)
    set_target_properties(arverb_shared PROPERTIES OUTPUT_NAME "arverb")
endif()

if(ARVERB_BUILD_TESTS)
    enable_testing()
    add_executable(arverb_tests tests/test_reverb.cpp)
    target_link_libraries(arverb_tests PRIVATE arverb_core)
    add_test(NAME arverb_tests COMMAND arverb_tests)
    add_executable(arverb_contract_tests tests/test_contract.cpp)
    target_link_libraries(arverb_contract_tests PRIVATE arverb_core)
    add_test(NAME arverb_contract_tests COMMAND arverb_contract_tests)
    add_executable(arverb_audio_regression_tests tests/test_audio_regression.cpp)
    target_link_libraries(arverb_audio_regression_tests PRIVATE arverb_core)
    add_test(NAME arverb_audio_regression_tests COMMAND arverb_audio_regression_tests)
    add_executable(arverb_stress_tests tests/test_stress.cpp)
    target_link_libraries(arverb_stress_tests PRIVATE arverb_core)
    add_test(NAME arverb_stress_tests COMMAND arverb_stress_tests)
    if(ARVERB_BUILD_SHARED)
        # Compile the consumer in ISO C to validate real dynamic library ABI
        # including Windows __declspec(dllimport) import-library generation.
        add_executable(arverb_c_abi_tests tests/test_c_abi.c)
        target_compile_features(arverb_c_abi_tests PRIVATE c_std_11)
        target_compile_definitions(arverb_c_abi_tests PRIVATE
            ARVERB_USE_SHARED ARVERB_EXPECTED_VERSION="${PROJECT_VERSION}")
        target_include_directories(arverb_c_abi_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_link_libraries(arverb_c_abi_tests PRIVATE arverb_shared)
        add_test(NAME arverb_c_abi_tests COMMAND arverb_c_abi_tests)
    endif()
    add_executable(arverb_benchmark tests/benchmark.cpp)
    target_link_libraries(arverb_benchmark PRIVATE arverb_core)
    # Golden audio fingerprints compared to the pre-optimization DSP on the
    # same Linux CI compiler. This is a test tool, never a plugin dependency.
    add_executable(arverb_audio_signature tests/audio_signature.cpp)
    target_link_libraries(arverb_audio_signature PRIVATE arverb_core)
    add_test(NAME arverb_audio_signature COMMAND arverb_audio_signature)
    add_executable(arverb_sonic_structure_tests tests/test_sonic_structure.cpp)
    target_link_libraries(arverb_sonic_structure_tests PRIVATE arverb_core)
    add_test(NAME arverb_sonic_structure_tests COMMAND arverb_sonic_structure_tests)
    add_executable(arverb_algorithm_tests tests/test_algorithms.cpp)
    target_link_libraries(arverb_algorithm_tests PRIVATE arverb_core)
    add_test(NAME arverb_algorithm_tests COMMAND arverb_algorithm_tests)
    add_executable(arverb_mix_air_tests tests/test_mix_air.cpp)
    target_link_libraries(arverb_mix_air_tests PRIVATE arverb_core)
    add_test(NAME arverb_mix_air_tests COMMAND arverb_mix_air_tests)
    add_executable(arverb_demo examples/impulse_wav.cpp)
    target_link_libraries(arverb_demo PRIVATE arverb_core)
endif()

# Opt-in: the reusable DSP library and its tests build without JUCE or network access.
if(ARVERB_BUILD_PLUGIN)
    if(CMAKE_VERSION VERSION_LESS "3.22")
        message(FATAL_ERROR "JUCE 9 needs CMake >= 3.22; core-only builds need >= 3.18")
    endif()
    if(ARVERB_JUCE_SOURCE_DIR)
        if(NOT EXISTS "${ARVERB_JUCE_SOURCE_DIR}/CMakeLists.txt")
            message(FATAL_ERROR "ARVERB_JUCE_SOURCE_DIR must point to a JUCE source tree")
        endif()
        add_subdirectory("${ARVERB_JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/juce" EXCLUDE_FROM_ALL)
    else()
        include(FetchContent)
        FetchContent_Declare(JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 9.0.4
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(JUCE)
    endif()

    # All tiny GUI resources are compiled into the JUCE frontend, never the
    # reusable C++ DSP core. The alpha logo is an optimized derivative of the
    # approved ArSon-Verb metallic PNG: 225x39 and 3.7 kB on disk.
    juce_add_binary_data(ArSonVerbFonts SOURCES
        assets/fonts/InterVariable.ttf
        assets/gui/ArSonVerbLogo.png
        assets/gui/ArSonVerbGoldKnob.png)

    juce_add_plugin(ARStudioReverb
        COMPANY_NAME "ArSon"
        BUNDLE_ID "com.arsonverb.plugin"
        PLUGIN_MANUFACTURER_CODE Arst
        PLUGIN_CODE Arvb
        PRODUCT_NAME "ArSonVerb"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        FORMATS VST3 Standalone)
    target_sources(ARStudioReverb PRIVATE
        plugins/juce_vst3/PluginProcessor.hpp
        plugins/juce_vst3/PluginProcessor.cpp
        plugins/juce_vst3/PluginEditor.hpp
        plugins/juce_vst3/PluginEditor.cpp)
    target_include_directories(ARStudioReverb PRIVATE plugins/juce_vst3)
    target_compile_features(ARStudioReverb PRIVATE cxx_std_17)
    target_compile_definitions(ARStudioReverb PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
    target_link_libraries(ARStudioReverb PRIVATE
        arverb_core
        ArSonVerbFonts
        juce::juce_audio_utils
        juce::juce_recommended_config_flags)

    # Independent host-contract test: loads real AudioProcessor code without a DAW.
    # The JUCE desktop SDK is only required in plugin-enabled CI, not core CI.
    if(ARVERB_BUILD_TESTS)
        add_executable(arverb_plugin_host_tests
            tests/test_plugin_host.cpp
            plugins/juce_vst3/PluginProcessor.cpp
            plugins/juce_vst3/PluginEditor.cpp)
        target_include_directories(arverb_plugin_host_tests PRIVATE plugins/juce_vst3)
        target_compile_features(arverb_plugin_host_tests PRIVATE cxx_std_17)
        target_compile_definitions(arverb_plugin_host_tests PRIVATE
            JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
        target_link_libraries(arverb_plugin_host_tests PRIVATE
            arverb_core ArSonVerbFonts
            juce::juce_audio_utils juce::juce_recommended_config_flags)
        add_test(NAME arverb_plugin_host_tests COMMAND arverb_plugin_host_tests)
    endif()
endif()

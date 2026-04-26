cmake_minimum_required(VERSION 3.16)
project(txtCounterModule LANGUAGES CXX)

add_library(txtCounterModule SHARED
    txtCounterModule.cpp
    txtCounterModule.h
    ${CMAKE_SOURCE_DIR}/src/core/extensions/ingestModuleInterface.h
    txtCounter.json
)

target_include_directories(txtCounterModule
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

find_package(Qt6 REQUIRED COMPONENTS Core)

target_link_libraries(txtCounterModule
    PRIVATE
        Qt6::Core
)

set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/plugins)
set_target_properties(txtCounterModule PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
)

if (WIN32)
    set_target_properties(txtCounterModule PROPERTIES PREFIX "")
endif()

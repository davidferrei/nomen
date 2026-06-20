cmake_minimum_required(VERSION 3.16)
project(yaraXModule LANGUAGES CXX)

add_library(yaraXModule SHARED
    yaraXModule.cpp
    yaraXModule.h
    yaraXScanner.cpp
    yaraXScanner.h
    yaraXMatchResultData.h
    ${CMAKE_SOURCE_DIR}/src/core/extensions/ingestModuleInterface.h
    yaraXModule.json
)

target_include_directories(yaraXModule
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/lib
        /usr/local/include
)

find_package(Qt6 REQUIRED COMPONENTS Core)

target_link_libraries(yaraXModule
    PRIVATE
        Qt6::Core
)

set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/plugins)

target_link_libraries(yaraXModule PRIVATE /usr/local/lib/libyara_x_capi.a)

set_target_properties(yaraXModule PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
)

if (WIN32)
    set_target_properties(yaraXModule PROPERTIES PREFIX "")
endif()

cmake_minimum_required(VERSION 3.16)
project(detectMIMEType LANGUAGES CXX)

add_library(detectMIMEType SHARED
    detectMIMEType.cpp
    detectMIMEType.h
    ${CMAKE_SOURCE_DIR}/src/core/extensions/ingestModuleInterface.h
    detectMIMEType.json
)

target_include_directories(detectMIMEType
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

find_package(Qt6 REQUIRED COMPONENTS Core)

target_link_libraries(detectMIMEType
    PRIVATE
        Qt6::Core
)

set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/plugins)
set_target_properties(detectMIMEType PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
)

if (WIN32)
    set_target_properties(detectMIMEType PROPERTIES PREFIX "")
endif()

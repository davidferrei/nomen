cmake_minimum_required(VERSION 3.16)

project(Nomen VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Threads)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# SleuthKit
find_library(SLEUTHKIT_LIB NAMES tsk tsk3 PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
find_path(SLEUTHKIT_INCLUDE_DIR NAMES libtsk.h PATHS /usr/include/tsk /usr/local/include/tsk /usr/include /usr/local/include)

if(NOT SLEUTHKIT_LIB OR NOT SLEUTHKIT_INCLUDE_DIR)
    message(WARNING "SleuthKit not Found. Attempting fallback to System Paths")
    set(SLEUTHKIT_INCLUDE_DIR /usr/include/tsk)
    set(SLEUTHKIT_LIB /usr/lib/libtsk.so)
endif()

include_directories(${SLEUTHKIT_INCLUDE_DIR})

# IBPP
add_library(ibpp STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/ibpp/core/all_in_one.cpp
)

# LMDB
add_library(lmdb STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/liblmdb/mdb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/liblmdb/midl.c
)

# Project Sources
file(GLOB_RECURSE PROJECT_SOURCES
    src/*.cpp
    src/*.h
    src/*.ui
)

target_include_directories(ibpp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/ibpp/core
)

target_compile_definitions(ibpp PRIVATE IBPP_LINUX)
target_compile_features(ibpp PRIVATE cxx_std_17)


target_include_directories(lmdb
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/liblmdb
)

# Qt Executable
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Nomen
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources/icons/icons.qrc
    )
else()
    add_executable(Nomen
        ${PROJECT_SOURCES}
        resources/icons/icons.qrc
    )
endif()

target_include_directories(Nomen
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SLEUTHKIT_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/lib
        ${CMAKE_SOURCE_DIR}/lib/ibpp/core
)

target_link_libraries(Nomen
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Threads::Threads
        ${SLEUTHKIT_LIB}
        ibpp
        lmdb
        ${CMAKE_SOURCE_DIR}/lib/ibpp/libfbclient.so
)

target_link_options(Nomen PRIVATE -rdynamic)

target_compile_definitions(Nomen PRIVATE IBPP_LINUX)
target_compile_features(ibpp PRIVATE cxx_std_17)

set_target_properties(Nomen PROPERTIES
    BUILD_RPATH "$ORIGIN/../lib"
    INSTALL_RPATH "$ORIGIN/../lib"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

# macOS / Windows Properties
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Nomen)
endif()

set_target_properties(Nomen PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Nomen
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Nomen)
endif()

##Extensions
add_subdirectory(extensions/ingestModules)

## Tests
add_subdirectory(tests)

cmake_minimum_required(VERSION 3.16)

project(Nomen VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(SLEUTHKIT_INCLUDE_DIR /usr/include/tsk)
# set(SLEUTHKIT_LIB /usr/lib/libsleuthkit.so)
set(SLEUTHKIT_LIB /usr/lib/libtsk.so)

include_directories(${SLEUTHKIT_INCLUDE_DIR})


file(GLOB_RECURSE PROJECT_SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/*.ui"
)
add_library(ibpp STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ibpp/core/all_in_one.cpp
)

#add_library(ibpp STATIC ${IBPP_SOURCES})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Nomen
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

    target_include_directories(Nomen
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src
            ${SLEUTHKIT_INCLUDE_DIR}
    )

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Nomen APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Nomen SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Nomen
            ${PROJECT_SOURCES}
        )
    endif()
endif()


target_include_directories(Nomen PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(Nomen PRIVATE IBPP_LINUX)
target_compile_features(ibpp PRIVATE cxx_std_17)

set_target_properties(Nomen PROPERTIES
    BUILD_RPATH "$ORIGIN/../lib"
    INSTALL_RPATH "$ORIGIN/../lib"
)


# target_link_libraries(Nomen PRIVATE Qt${QT_VERSION_MAJOR}::Widgets /usr/lib/libtsk.so)
target_link_libraries(Nomen PRIVATE Qt${QT_VERSION_MAJOR}::Widgets tsk ibpp ${CMAKE_SOURCE_DIR}/lib/libfbclient.so)

set(CMAKE_BUILD_RPATH "$ORIGIN/firebird/lib")
set(CMAKE_INSTALL_RPATH "$ORIGIN/firebird/lib")



# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
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

cmake_minimum_required(VERSION 3.20)
project(BlinkMD VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 6.9+ uses C++20 constexpr MOC helpers that require MSVC 2022 (cl 19.30+)
if(MSVC AND MSVC_VERSION LESS 1930)
    message(FATAL_ERROR
        "MSVC 2022 or later is required (detected: MSVC ${MSVC_VERSION}).\n"
        "Qt 6.9+ dropped support for MSVC 2019. "
        "Please install Visual Studio 2022 or the VS 2022 Build Tools:\n"
        "  https://visualstudio.microsoft.com/downloads/")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    resources.qrc
)

if(WIN32)
    # Windows resource file for application icon (blinkmd.ico generated at build time)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/blinkmd.ico")
        list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
    endif()
    add_executable(BlinkMD WIN32 ${SOURCES})
elseif(APPLE)
    set(MACOSX_ICON "${CMAKE_CURRENT_SOURCE_DIR}/blinkmd.icns")
    if(EXISTS "${MACOSX_ICON}")
        set_source_files_properties("${MACOSX_ICON}" PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources"
        )
        list(APPEND SOURCES "${MACOSX_ICON}")
    endif()
    add_executable(BlinkMD MACOSX_BUNDLE ${SOURCES})
    set_target_properties(BlinkMD PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.blinkmd.app"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        MACOSX_BUNDLE_ICON_FILE "blinkmd.icns"
    )
else()
    add_executable(BlinkMD ${SOURCES})
endif()

target_link_libraries(BlinkMD PRIVATE Qt6::Widgets)

install(TARGETS BlinkMD
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

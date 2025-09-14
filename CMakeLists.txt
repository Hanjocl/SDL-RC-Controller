cmake_minimum_required(VERSION 3.16)
project(SDL_Controller_Lib)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)

# Qt for API
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Quick Qml)

# -----------------------------
# Library: SDL_Controller_Lib
# -----------------------------
set(LIB_NAME "SDL_Controller_Lib")
add_library(${LIB_NAME} STATIC
    src/QmlControllerApi.cpp
    src/QmlControllerApi.h
)

# Include directories
target_include_directories(${LIB_NAME} 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(${LIB_NAME} 
    PUBLIC SDL2::SDL2
    PUBLIC Qt6::Core
    PUBLIC Qt6::Widgets
    PUBLIC Qt6::Gui
    PUBLIC Qt6::Quick
    PUBLIC Qt6::Qml
)

# QML module for library
qt_add_qml_module(${LIB_NAME}
    URI SDL-Controller
    VERSION 1.0
    QML_FILES qml/Main.qml
)

# -----------------------------
# Executable: example app
# -----------------------------
qt_add_executable(${PROJECT_NAME}
    example/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE ${LIB_NAME}
)

# Qt policies
qt_policy(SET QTP0004 NEW)
qt_policy(SET QTP0001 OLD)

# -----------------------------
# Windows deployment
# -----------------------------
if(WIN32)
    find_program(QT_WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${QT_WINDEPLOYQT_EXECUTABLE} 
            --qmldir ${CMAKE_SOURCE_DIR}/qml
            $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Running windeployqt with custom options"
    )
endif()

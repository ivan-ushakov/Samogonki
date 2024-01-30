cmake_minimum_required(VERSION 3.20)

project(EDITOR VERSION 1.0.0 LANGUAGES C CXX)

SET(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")

set(APPLICATION_NAME Editor)

find_package(ZLIB REQUIRED)
find_package(Iconv REQUIRED)

include_directories(3rdparty/SDL-2.26.5/include)
link_directories(3rdparty/SDL-2.26.5/lib)

set(APPLICATION_SOURCES
    Editor/main.cpp
)

if (APPLE)
    add_executable(${APPLICATION_NAME} MACOSX_BUNDLE ${APPLICATION_SOURCES})
    set_property(TARGET ${APPLICATION_NAME} PROPERTY MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Editor/cMake/Info.plist.in")
    target_compile_options(${APPLICATION_NAME} PRIVATE -x objective-c++)
    target_link_libraries(${APPLICATION_NAME} PRIVATE stdc++
            "-framework CoreFoundation" "-framework Cocoa" "-framework QuartzCore" "-framework OpenGL" 
            "-framework CoreAudio" "-framework AudioToolbox" "-framework ForceFeedback" "-framework CoreHaptics"
            "-framework GameController" "-framework IOKit" "-framework Carbon" "-framework Metal"
            objc ZLIB::ZLIB Iconv::Iconv SDL2)
endif(APPLE)

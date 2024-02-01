cmake_minimum_required(VERSION 3.20)

project(EDITOR VERSION 1.0.0 LANGUAGES C CXX)

SET(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")

set(APPLICATION_NAME Editor)

find_package(ZLIB REQUIRED)
find_package(Iconv REQUIRED)

include_directories(3rdparty/SDL-2.26.5/include/SDL2)
link_directories(3rdparty/SDL-2.26.5/lib)

include_directories(3rdparty/imgui)
include_directories(3rdparty/imgui/backends)

include_directories(MechoSoma/aciparser)
include_directories(MechoSoma/XTool)

set(APPLICATION_SOURCES
    Editor/main.cpp
    MechoSoma/aciparser/aci_parser.cpp
    MechoSoma/aciparser/aci_parser.h
    MechoSoma/XTool/filesystem.cpp
    MechoSoma/XTool/inzip.cpp
    MechoSoma/XTool/itoa.cpp
    MechoSoma/XTool/outzip.cpp
    MechoSoma/XTool/treezip.cpp
    MechoSoma/XTool/xbcnvin.cpp
    MechoSoma/XTool/xbcnvout.cpp
    MechoSoma/XTool/xbcore.cpp
    MechoSoma/XTool/xerrhand.cpp
    MechoSoma/XTool/xstream.cpp
    # ImGui
    3rdparty/imgui/imgui.cpp
    3rdparty/imgui/imgui_draw.cpp
    3rdparty/imgui/imgui_tables.cpp
    3rdparty/imgui/imgui_widgets.cpp
    3rdparty/imgui/backends/imgui_impl_sdl2.cpp
    3rdparty/imgui/backends/imgui_impl_sdlrenderer2.cpp
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

set_target_properties(${APPLICATION_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

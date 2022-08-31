cmake_minimum_required(VERSION 3.16)
project("mgnr")

#set(CMAKE_CXX_FLAGS "-std=c++20 -ggdb")
set(CMAKE_CXX_FLAGS "-std=c++20 -ggdb")

find_package(SDL2 2.0.17 REQUIRED)
find_path(sdl2_INCLUDE_DIR SDL.h)
find_library(sdl2_LIBRARY SDL2)
find_library(sdl2main_LIBRARY SDL2main)
find_library(fluidsynth_LIBRARY fluidsynth)
find_path(sdl2_ttf_INCLUDE_DIR SDL_ttf.h)
find_library(sdl2_ttf_LIBRARY SDL2_ttf)

add_subdirectory(modules)

include_directories(
    ./
    ./mgenner
    ./utils
    ./imgui
    ./imgui/backends
    ./Dear-ImGui-Lua-Binding/include
    ./Dear-ImGui-Lua-Binding/src
    /usr/local/include/SDL2 #imgui的头文件路径有问题，必须加这一行。可根据实际情况修改
)

add_executable(mgnr
    ./view/main.cpp
    ./view/midiEditor.cpp
    ./view/ui.cpp
    ./view/events.cpp
    ./view/synth.cpp
    ./view/plugin.cpp

    ./mgenner/Options.cpp
    ./mgenner/midiMap.cpp
    ./mgenner/Binasc.cpp
    ./mgenner/offline.cpp
    ./mgenner/kalman.cpp
    ./mgenner/stringPool.cpp
    ./mgenner/MidiEvent.cpp
    ./mgenner/MidiMessage.cpp
    ./mgenner/midisplit.cpp
    ./mgenner/synthOutput.cpp
    ./mgenner/MidiEventList.cpp
    ./mgenner/midiLoader.cpp
    ./mgenner/hbb.cpp
    ./mgenner/MidiFile.cpp
    ./mgenner/player.cpp
    ./mgenner/synth.cpp
    ./mgenner/editTable.cpp

    ./lua/lapi.c
    ./lua/lauxlib.c
    ./lua/lbaselib.c
    ./lua/lcode.c
    ./lua/lcorolib.c
    ./lua/lctype.c
    ./lua/ldblib.c
    ./lua/ldebug.c
    ./lua/ldo.c
    ./lua/ldump.c
    ./lua/lfunc.c
    ./lua/lgc.c
    ./lua/linit.c
    ./lua/liolib.c
    ./lua/llex.c
    ./lua/lmathlib.c
    ./lua/lmem.c
    ./lua/loadlib.c
    ./lua/lobject.c
    ./lua/lopcodes.c
    ./lua/loslib.c
    ./lua/lparser.c
    ./lua/lstate.c
    ./lua/lstring.c
    ./lua/lstrlib.c
    ./lua/ltable.c
    ./lua/ltablib.c
    ./lua/ltm.c
    ./lua/lundump.c
    ./lua/lutf8lib.c
    ./lua/lvm.c
    ./lua/lzio.c

    ./imgui/imgui.cpp
    ./imgui/imgui_draw.cpp
    ./imgui/imgui_tables.cpp
    ./imgui/imgui_widgets.cpp
    ./imgui/imgui_demo.cpp
    ./imgui/backends/imgui_impl_sdl.cpp
    ./imgui/backends/imgui_impl_sdlrenderer.cpp

    ./Dear-ImGui-Lua-Binding/src/lua_imgui_enum.cpp
    ./Dear-ImGui-Lua-Binding/src/lua_imgui_hash.cpp
    ./Dear-ImGui-Lua-Binding/src/lua_imgui_map.cpp
    ./Dear-ImGui-Lua-Binding/src/lua_imgui_ImGui.cpp
    ./Dear-ImGui-Lua-Binding/src/lua_imgui_ImGuiTextBuffer.cpp
    ./Dear-ImGui-Lua-Binding/src/lua_imgui_type.cpp
    ./Dear-ImGui-Lua-Binding/src/lua_imgui.cpp
)

target_link_libraries(mgnr
    ${SDL2_LIBRARIES}
    ${sdl2_ttf_LIBRARY}
    ${fluidsynth_LIBRARY}
)

set_target_properties(mgnr PROPERTIES OUT_NAME "mgnr")

file(COPY config.lua DESTINATION ./)
file(COPY res DESTINATION ./)

cmake_minimum_required(VERSION 3.16)
project("mgnr")

#set(CMAKE_CXX_FLAGS "-std=c++20 -ggdb")
set(CMAKE_CXX_FLAGS "-std=c++20 -o3 -DLUA_USE_DLOPEN")
set(CMAKE_C_FLAGS "-DLUA_USE_DLOPEN")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-E -pthread")

find_package(SDL2 2.0.17 REQUIRED)
find_path(sdl2_INCLUDE_DIR SDL.h)
find_library(sdl2_LIBRARY SDL2)
find_library(sdl2main_LIBRARY SDL2main)
find_library(fluidsynth_LIBRARY fluidsynth)
find_path(sdl2_ttf_INCLUDE_DIR SDL_ttf.h)
find_library(sdl2_ttf_LIBRARY SDL2_ttf)
find_library(sox_LIBRARY sox)
find_library(dl_LIBRARY dl)

add_subdirectory(modules)

include_directories(
    ./
    ./mgenner
    ./utils
    ./sysmodule
    ./dep/
    ./dep/imgui
    ./dep/imnodes
    ./dep/imgui/backends
    ./dep/Dear-ImGui-Lua-Binding/include
    ./dep/Dear-ImGui-Lua-Binding/src
    /usr/local/include/SDL2 #imgui的头文件路径有问题，必须加这一行。可根据实际情况修改
)

add_executable(mgnr
    ./view/main.cpp
    ./view/midiEditor.cpp
    ./view/ui.cpp
    ./view/events.cpp
    ./view/module.cpp
    ./view/vscript.cpp
    ./view/renderContext.cpp
    ./view/editWindow.cpp
    ./view/offlineRender.cpp
    ./view/synthConfig.cpp

    ./vscript/vscript.cpp
    ./vscript/vscript_ui.cpp

    ./sysmodule/types/wavfile.cpp

    ./synthesizer/synthesizer.cpp
    ./synthesizer/effector.cpp
    ./synthesizer/vinstrument.cpp
    ./synthesizer/module/sf2.cpp

    ./utils/WavFile.cpp

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

    ./dep/lua/lapi.c
    ./dep/lua/lauxlib.c
    ./dep/lua/lbaselib.c
    ./dep/lua/lcode.c
    ./dep/lua/lcorolib.c
    ./dep/lua/lctype.c
    ./dep/lua/ldblib.c
    ./dep/lua/ldebug.c
    ./dep/lua/ldo.c
    ./dep/lua/ldump.c
    ./dep/lua/lfunc.c
    ./dep/lua/lgc.c
    ./dep/lua/linit.c
    ./dep/lua/liolib.c
    ./dep/lua/llex.c
    ./dep/lua/lmathlib.c
    ./dep/lua/lmem.c
    ./dep/lua/loadlib.c
    ./dep/lua/lobject.c
    ./dep/lua/lopcodes.c
    ./dep/lua/loslib.c
    ./dep/lua/lparser.c
    ./dep/lua/lstate.c
    ./dep/lua/lstring.c
    ./dep/lua/lstrlib.c
    ./dep/lua/ltable.c
    ./dep/lua/ltablib.c
    ./dep/lua/ltm.c
    ./dep/lua/lundump.c
    ./dep/lua/lutf8lib.c
    ./dep/lua/lvm.c
    ./dep/lua/lzio.c

    ./dep/imgui/imgui.cpp
    ./dep/imgui/imgui_draw.cpp
    ./dep/imgui/imgui_tables.cpp
    ./dep/imgui/imgui_widgets.cpp
    ./dep/imgui/imgui_demo.cpp
    ./dep/imgui/backends/imgui_impl_sdl.cpp
    ./dep/imgui/backends/imgui_impl_sdlrenderer.cpp

    ./dep/imnodes/imnodes.cpp

    ./dep/Dear-ImGui-Lua-Binding/src/lua_imgui_enum.cpp
    ./dep/Dear-ImGui-Lua-Binding/src/lua_imgui_hash.cpp
    ./dep/Dear-ImGui-Lua-Binding/src/lua_imgui_map.cpp
    ./dep/Dear-ImGui-Lua-Binding/src/lua_imgui_ImGui.cpp
    ./dep/Dear-ImGui-Lua-Binding/src/lua_imgui_ImGuiTextBuffer.cpp
    ./dep/Dear-ImGui-Lua-Binding/src/lua_imgui_type.cpp
    ./dep/Dear-ImGui-Lua-Binding/src/lua_imgui.cpp
)

target_link_libraries(mgnr
    ${SDL2_LIBRARIES}
    ${sdl2_ttf_LIBRARY}
    ${fluidsynth_LIBRARY}
    ${dl_LIBRARY}
    ${sox_LIBRARY}
)

set_target_properties(mgnr PROPERTIES OUT_NAME "mgnr")

file(COPY config.lua DESTINATION ./)
file(COPY res DESTINATION ./)

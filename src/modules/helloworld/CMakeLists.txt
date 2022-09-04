cmake_minimum_required(VERSION 3.16)
project("mgnr-modules-helloworld")

#add_definitions("-E")

include_directories(
    ./
    ../../
    ../../mgenner
    ../../utils
    ../../dep/
    ../../dep/imgui
    ../../dep/imgui/backends
    ../../dep/Dear-ImGui-Lua-Binding/include
    ../../dep/Dear-ImGui-Lua-Binding/src
    /usr/local/include/SDL2 #imgui的头文件路径有问题，必须加这一行。可根据实际情况修改
)

add_library(helloworld SHARED 
    helloworld.cpp)
set_target_properties(helloworld PROPERTIES OUT_NAME "helloworld")

file(COPY helloworld.lua DESTINATION ./)
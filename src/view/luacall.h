#pragma once
#include "lua.hpp"

//封装lua函数调用
inline void lua_callfunction_arg(lua_State* L, int num) {
    if (lua_pcall(L, num, 0, 0)) {
        auto err = lua_tostring(L, -1);
        printf("mgennerPlugin:error:%s\n", err);
    }
}

template <typename... Args>
inline void lua_callfunction_arg(lua_State* L, int num, int integer, Args... args) {
    lua_pushinteger(L, integer);
    lua_callfunction_arg(L, num + 1, args...);
}

template <typename... Args>
inline void lua_callfunction_arg(lua_State* L, int num, const char* str, Args... args) {
    lua_pushstring(L, str);
    lua_callfunction_arg(L, num + 1, args...);
}

template <typename... Args>
inline void lua_callfunction_arg(lua_State* L, int num, void* ptr, Args... args) {
    lua_pushlightuserdata(L, ptr);
    lua_callfunction_arg(L, num + 1, args...);
}

template <typename... Args>
inline void lua_callfunction(lua_State* L, Args... args) {
    lua_callfunction_arg(L, 0, args...);
}

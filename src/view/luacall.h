#pragma once
#include "lua.hpp"

//封装lua函数调用
inline int lua_callfunction_arg(lua_State* L, int num) {
    auto res = lua_pcall(L, num, 0, 0);
    if (res) {
        auto err = lua_tostring(L, -1);
        printf("mgennerPlugin:error:%s\n", err);
    }
    return res;
}

template <typename... Args>
inline int lua_callfunction_arg(lua_State* L, int num, int integer, Args... args) {
    lua_pushinteger(L, integer);
    return lua_callfunction_arg(L, num + 1, args...);
}

template <typename... Args>
inline int lua_callfunction_arg(lua_State* L, int num, const char* str, Args... args) {
    lua_pushstring(L, str);
    return lua_callfunction_arg(L, num + 1, args...);
}

template <typename... Args>
inline int lua_callfunction_arg(lua_State* L, int num, void* ptr, Args... args) {
    lua_pushlightuserdata(L, ptr);
    return lua_callfunction_arg(L, num + 1, args...);
}

template <typename... Args>
inline int lua_callfunction(lua_State* L, Args... args) {
    return lua_callfunction_arg(L, 0, args...);
}

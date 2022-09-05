#include "lua_imgui.hpp"
#include "mgenner.h"
void mgenner::loadConfig() {
    lua_mainthread = luaL_newstate();
    luaL_openlibs(lua_mainthread);
    luaopen_imgui(lua_mainthread);
    luaL_dofile(lua_mainthread, "config.lua");
    {
        lua_getglobal(lua_mainthread, "path_sf2");
        if (lua_isstring(lua_mainthread, -1)) {
            path_sf2 = lua_tostring(lua_mainthread, -1);
        }
        lua_pop(lua_mainthread, 1);
    }
    {
        lua_getglobal(lua_mainthread, "path_font");
        if (lua_isstring(lua_mainthread, -1)) {
            path_font = lua_tostring(lua_mainthread, -1);
        }
        lua_pop(lua_mainthread, 1);
    }
    std::vector<std::string> modulePath;
    {
        lua_pushcfunction(lua_mainthread, [](lua_State* L) -> int {
            //self, cmd
            if (!lua_islightuserdata(L, 1)) {
                return 0;
            }
            auto self = (mgenner*)lua_touserdata(L, 1);
            if (self == nullptr) {
                return 0;
            }
            if (lua_istable(L, 2)) {
                std::string name;

                lua_pushstring(L, "name");
                lua_gettable(L, -2);
                if (lua_isstring(L, -1)) {
                    name = lua_tostring(L, -1);
                }
                lua_pop(L, 1);

                if (!name.empty()) {
                    printf("mgennerModule:load:%s\n", name.c_str());

                    auto p = new moduleConfig;
                    p->name = name;

                    lua_pushstring(L, "init");
                    lua_gettable(L, -2);
                    if (lua_isfunction(L, -1)) {
                        p->init = luaL_ref(L, LUA_REGISTRYINDEX);
                    } else {
                        lua_pop(L, 1);
                    }

                    lua_pushstring(L, "shutdown");
                    lua_gettable(L, -2);
                    if (lua_isfunction(L, -1)) {
                        p->shutdown = luaL_ref(L, LUA_REGISTRYINDEX);
                    } else {
                        lua_pop(L, 1);
                    }

                    lua_pushstring(L, "drawUI");
                    lua_gettable(L, -2);
                    if (lua_isfunction(L, -1)) {
                        p->drawUI = luaL_ref(L, LUA_REGISTRYINDEX);
                    } else {
                        lua_pop(L, 1);
                    }

                    lua_pushstring(L, "loop");
                    lua_gettable(L, -2);
                    if (lua_isfunction(L, -1)) {
                        p->loop = luaL_ref(L, LUA_REGISTRYINDEX);
                        self->modules_loop.push_back(p);
                    } else {
                        lua_pop(L, 1);
                    }

                    self->modules.push_back(p);
                }
            }
            return 0;
        });
        lua_setglobal(lua_mainthread, "registerModule");

        lua_getglobal(lua_mainthread, "modulesInit");
        if (lua_isfunction(lua_mainthread, -1)) {
            //lua_pushlightuserdata(lua_mainthread, this);
            //if (lua_pcall(lua_mainthread, 1, 0, 0)) {
            //    auto err = lua_tostring(lua_mainthread, -1);
            //    printf("mgennermodule:error:%s\n", err);
            //}
            lua_callfunction(lua_mainthread, this);
        }
        lua_settop(lua_mainthread, 0);
    }
    //调用初始化函数
    for (auto it : modules) {
        if (it->init >= 0) {
            lua_rawgeti(lua_mainthread, LUA_REGISTRYINDEX, it->init);
            if (lua_isfunction(lua_mainthread, -1)) {
                //lua_pushlightuserdata(lua_mainthread, this);
                //if (lua_pcall(lua_mainthread, 1, 0, 0)) {
                //    auto err = lua_tostring(lua_mainthread, -1);
                //    printf("mgennermodule:init %s:error:%s\n", it->name.c_str(), err);
                //}
                lua_callfunction(lua_mainthread, this);
            }
            lua_settop(lua_mainthread, 0);
        }
    }
    vscript_init();
}

void mgenner::module_menu() {
    if (ImGui::MenuItem("节点控制台")) {
        module_showNodeEditor = true;
    }
    ImGui::MenuItem("扩展配置", NULL, false, false);
    for (auto it : modules) {
        if (ImGui::MenuItem(it->name.c_str())) {
            modules_showing.insert(it);
        }
    }
}
void mgenner::module_show() {
    std::vector<moduleConfig*> rmlist;
    for (auto it : modules_showing) {
        bool showing = true;
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(it->name.c_str(), &showing)) {
            checkfocus();
            if (it->drawUI >= 0) {
                lua_rawgeti(lua_mainthread, LUA_REGISTRYINDEX, it->drawUI);
                if (lua_isfunction(lua_mainthread, -1)) {
                    lua_callfunction(lua_mainthread, this);
                }
                lua_settop(lua_mainthread, 0);
            }
            ImGui::End();
        }
        if (!showing) {
            rmlist.push_back(it);
        }
    }
    for (auto it : rmlist) {
        modules_showing.erase(it);
    }

    module_nodeEditor();
}
void mgenner::module_loop() {
    for (auto it : modules_loop) {
        if (it->loop >= 0) {
            lua_rawgeti(lua_mainthread, LUA_REGISTRYINDEX, it->loop);
            if (lua_isfunction(lua_mainthread, -1)) {
                lua_callfunction(lua_mainthread, this);
            }
            lua_settop(lua_mainthread, 0);
        }
    }

    vscript.exec_loop();
    for (int i = 0; i < 64; ++i) {
        if (vscript.exec_running()) {
            vscript.exec_step();
        } else {
            break;
        }
    }
}
void mgenner::shutdownModules() {
    for (auto it : modules) {
        if (it->shutdown >= 0) {
            lua_rawgeti(lua_mainthread, LUA_REGISTRYINDEX, it->shutdown);
            if (lua_isfunction(lua_mainthread, -1)) {
                lua_callfunction(lua_mainthread, this);
            }
            lua_settop(lua_mainthread, 0);
        }
        delete it;
    }
    lua_close(lua_mainthread);
    ImNodes::DestroyContext();
}

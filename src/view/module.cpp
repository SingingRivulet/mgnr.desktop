#include "lua_imgui.hpp"
#include "renderContext.h"

struct node_lua : public mgnr::vscript::node_ui {
    renderContext* global;
    renderContext::vclass_t* vclass;
    node_lua() {
    }
    void exec() override {
        if (vclass->exec >= 0) {
            lua_rawgeti(global->lua_mainthread, LUA_REGISTRYINDEX, vclass->exec);
            if (lua_isfunction(global->lua_mainthread, -1)) {
                auto res = lua_callfunction(global->lua_mainthread, global, this);
                if (res) {
                    auto err = lua_tostring(global->lua_mainthread, -1);
                    global->scriptConsole.push_back(std::string("脚本错误") + err);
                }
            }
            lua_settop(global->lua_mainthread, 0);
        }
    }
    void draw() override {
        node_ui::draw();
        if (vclass->draw >= 0) {
            lua_rawgeti(global->lua_mainthread, LUA_REGISTRYINDEX, vclass->draw);
            if (lua_isfunction(global->lua_mainthread, -1)) {
                auto res = lua_callfunction(global->lua_mainthread, global, this);
                if (res) {
                    auto err = lua_tostring(global->lua_mainthread, -1);
                    global->scriptConsole.push_back(std::string("脚本错误") + err);
                }
            }
            lua_settop(global->lua_mainthread, 0);
        }
    }
    ~node_lua() override {}
};

static int lua_registerModule(lua_State* L) {
    //self, cmd
    if (!lua_islightuserdata(L, 1)) {
        return 0;
    }
    auto self = (renderContext*)lua_touserdata(L, 1);
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

            auto p = new renderContext::moduleConfig;
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
                self->modules_haveUI.push_back(p);
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

            lua_pushstring(L, "vscript");
            lua_gettable(L, -2);
            if (lua_istable(L, -1)) {  //是模块
                std::unique_ptr<renderContext::vclass_t> vclass_p(new renderContext::vclass_t);
                vclass_p->name = name;
                lua_pushstring(L, "input");
                lua_gettable(L, -2);
                if (lua_istable(L, -1)) {
                    int len = luaL_len(L, -1);
                    for (int i = 1; i <= len; ++i) {
                        lua_geti(L, -1, i);
                        if (lua_istable(L, -1)) {
                            std::string port_name;
                            std::string port_type;
                            lua_pushstring(L, "name");
                            lua_gettable(L, -2);
                            if (lua_isstring(L, -1)) {
                                port_name = lua_tostring(L, -1);
                            }
                            lua_pop(L, 1);

                            lua_pushstring(L, "type");
                            lua_gettable(L, -2);
                            if (lua_isstring(L, -1)) {
                                port_type = lua_tostring(L, -1);
                            }
                            lua_pop(L, 1);

                            if (!port_name.empty()) {
                                vclass_p->inputs.push_back(std::make_tuple(port_name, port_type));
                            }
                        }
                        lua_pop(L, 1);
                    }
                }
                lua_pop(L, 1);

                lua_pushstring(L, "output");
                lua_gettable(L, -2);
                if (lua_istable(L, -1)) {
                    int len = luaL_len(L, -1);
                    for (int i = 1; i <= len; ++i) {
                        lua_geti(L, -1, i);
                        if (lua_istable(L, -1)) {
                            std::string port_name;
                            std::string port_type;
                            lua_pushstring(L, "name");
                            lua_gettable(L, -2);
                            if (lua_isstring(L, -1)) {
                                port_name = lua_tostring(L, -1);
                            }
                            lua_pop(L, 1);

                            lua_pushstring(L, "type");
                            lua_gettable(L, -2);
                            if (lua_isstring(L, -1)) {
                                port_type = lua_tostring(L, -1);
                            }
                            lua_pop(L, 1);

                            if (!port_name.empty()) {
                                vclass_p->outputs.push_back(std::make_tuple(port_name, port_type));
                            }
                        }
                        lua_pop(L, 1);
                    }
                }
                lua_pop(L, 1);

                lua_pushstring(L, "draw");
                lua_gettable(L, -2);
                if (lua_isfunction(L, -1)) {
                    vclass_p->draw = luaL_ref(L, LUA_REGISTRYINDEX);
                } else {
                    lua_pop(L, 1);
                }

                lua_pushstring(L, "exec");
                lua_gettable(L, -2);
                if (lua_isfunction(L, -1)) {
                    vclass_p->exec = luaL_ref(L, LUA_REGISTRYINDEX);
                } else {
                    lua_pop(L, 1);
                }
                lua_pushstring(L, "needFullInput");
                lua_gettable(L, -2);
                if (lua_isboolean(L, -1)) {
                    vclass_p->needFullInput = lua_toboolean(L, -1);
                }
                lua_pop(L, 1);

                //创建节点类
                auto vpp = vclass_p.get();
                auto vclass_callback = std::tuple<
                    std::string,
                    std::function<mgnr::vscript::node*(renderContext::vscript_t*)>>(
                    vpp->name,
                    [vpp, self](renderContext::vscript_t* s) {
                        node_lua* nl = new node_lua;
                        nl->global = self;
                        nl->vclass = vpp;
                        nl->name = vpp->name;
                        //构建输入输出
                        for (auto& it : vpp->inputs) {
                            std::unique_ptr<mgnr::vscript::port_input> in0(
                                new mgnr::vscript::port_input);
                            in0->name = std::get<0>(it);
                            in0->type = std::get<1>(it);
                            nl->input.push_back(std::move(in0));
                        }
                        for (auto& it : vpp->outputs) {
                            std::unique_ptr<mgnr::vscript::port_output> out0(
                                new mgnr::vscript::port_output);
                            out0->name = std::get<0>(it);
                            out0->type = std::get<1>(it);
                            nl->output.push_back(std::move(out0));
                        }
                        nl->needFullInput = vpp->needFullInput;
                        std::unique_ptr<mgnr::vscript::node> n(nl);
                        auto p = s->addNode(std::move(n));
                        ImNodes::SetNodeScreenSpacePos(p->id, ImGui::GetMousePos());
                        ImNodes::SnapNodeToGrid(p->id);
                        return p;
                    });

                lua_pushstring(L, "menu");
                lua_gettable(L, -2);
                if (lua_isstring(L, -1)) {
                    self->vscript.scriptClass[lua_tostring(L, -1)].push_back(vclass_callback);
                } else {
                    self->vscript.scriptClass_other.push_back(vclass_callback);
                }
                lua_pop(L, 1);

                p->vclass = std::move(vclass_p);
            }
            lua_pop(L, 1);

            self->modules.push_back(p);
        }
    }
    return 0;
}

static int lua_vscript_getInput_string(lua_State* L) {
    //self,port
    //从端口获取一个值，同时将其删除
    if (!lua_islightuserdata(L, 1)) {
        return 0;
    }
    auto self = (node_lua*)lua_touserdata(L, 1);
    if (self == nullptr) {
        return 0;
    }
    int portId = luaL_checkinteger(L, 2);
    try {
        auto& port = self->input.at(portId);
        if (port->data.size() > 0) {
            std::shared_ptr<mgnr::vscript::value> d = port->data.front();
            try {
                auto ptr = std::dynamic_pointer_cast<mgnr::vscript::value_string>(d);
                if (ptr == nullptr) {
                    return luaL_argerror(L, 2, "获取值失败");
                }
                lua_pushstring(L, ptr->data.c_str());
                port->data.pop_front();
                return 1;
            } catch (...) {
                return luaL_argerror(L, 2, "获取值失败");
            }
        } else {
            return luaL_argerror(L, 2, "缓冲区为空");
        }

    } catch (...) {
    }

    lua_pushboolean(L, false);
    return 1;
}
static int lua_vscript_getInput_int(lua_State* L) {
    //self,port
    //从端口获取一个值，同时将其删除
    if (!lua_islightuserdata(L, 1)) {
        return 0;
    }
    auto self = (node_lua*)lua_touserdata(L, 1);
    if (self == nullptr) {
        return 0;
    }
    int portId = luaL_checkinteger(L, 2);
    try {
        auto& port = self->input.at(portId);
        if (port->data.size() > 0) {
            std::shared_ptr<mgnr::vscript::value> d = port->data.front();
            try {
                auto ptr = std::dynamic_pointer_cast<mgnr::vscript::value_int>(d);
                if (ptr == nullptr) {
                    return luaL_argerror(L, 2, "获取值失败");
                }
                lua_pushinteger(L, ptr->data);
                port->data.pop_front();
                return 1;
            } catch (...) {
                return luaL_argerror(L, 2, "获取值失败");
            }
        } else {
            return luaL_argerror(L, 2, "缓冲区为空");
        }

    } catch (...) {
    }

    lua_pushboolean(L, false);
    return 1;
}

static int lua_vscript_getInputLen(lua_State* L) {
    //self,port
    //获取端口值的数量
    if (!lua_islightuserdata(L, 1)) {
        return 0;
    }
    auto self = (node_lua*)lua_touserdata(L, 1);
    if (self == nullptr) {
        return 0;
    }
    int portId = luaL_checkinteger(L, 2);
    try {
        auto& port = self->input.at(portId);
        lua_pushinteger(L, port->data.size());
        return 1;
    } catch (...) {
    }
    lua_pushboolean(L, false);
    return 1;
}

static int lua_vscript_send(lua_State* L) {
    //self,port,data
    //向端口发送一个值
    if (!lua_islightuserdata(L, 1)) {
        return 0;
    }
    auto self = (node_lua*)lua_touserdata(L, 1);
    if (self == nullptr) {
        return 0;
    }
    int portId = luaL_checkinteger(L, 2);
    try {
        auto& port = self->output.at(portId);
        bool status = false;
        if (lua_isstring(L, 3)) {
            auto d = std::make_shared<mgnr::vscript::value_string>();
            d->data = std::string(lua_tostring(L, 3));
            port->send(d);
        } else if (lua_isinteger(L, 3)) {
            auto d = std::make_shared<mgnr::vscript::value_int>();
            d->data = (int)lua_tointeger(L, 3);
            port->send(d);
        }
        lua_pushboolean(L, status);
        return 1;
    } catch (...) {
    }
    lua_pushboolean(L, false);
    return 1;
}

static int lua_vscript_print(lua_State* L) {
    //self,data
    //显示
    if (!lua_islightuserdata(L, 1)) {
        return 0;
    }
    auto self = (node_lua*)lua_touserdata(L, 1);
    if (self == nullptr) {
        return 0;
    }
    if (lua_isstring(L, 2)) {
        self->global->scriptConsole.push_back(lua_tostring(L, 2));
    }
    return 0;
}

void renderContext::loadConfig() {
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
        lua_pushcfunction(lua_mainthread, lua_registerModule);
        lua_setglobal(lua_mainthread, "registerModule");

        {
            lua_newtable(lua_mainthread);

            lua_pushstring(lua_mainthread, "getInput_string");
            lua_pushcfunction(lua_mainthread, lua_vscript_getInput_string);
            lua_settable(lua_mainthread, -3);

            lua_pushstring(lua_mainthread, "getInput_int");
            lua_pushcfunction(lua_mainthread, lua_vscript_getInput_int);
            lua_settable(lua_mainthread, -3);

            lua_pushstring(lua_mainthread, "getInputLen");
            lua_pushcfunction(lua_mainthread, lua_vscript_getInputLen);
            lua_settable(lua_mainthread, -3);

            lua_pushstring(lua_mainthread, "send");
            lua_pushcfunction(lua_mainthread, lua_vscript_send);
            lua_settable(lua_mainthread, -3);

            lua_pushstring(lua_mainthread, "print");
            lua_pushcfunction(lua_mainthread, lua_vscript_print);
            lua_settable(lua_mainthread, -3);

            lua_setglobal(lua_mainthread, "vscript");
        }

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

void renderContext::module_menu() {
    if (ImGui::MenuItem("节点控制台")) {
        module_showNodeEditor = true;
    }
    ImGui::MenuItem("扩展配置", NULL, false, false);
    for (auto it : modules_haveUI) {
        if (ImGui::MenuItem(it->name.c_str())) {
            modules_showing.insert(it);
        }
    }
}
void renderContext::module_show() {
    std::vector<moduleConfig*> rmlist;
    for (auto it : modules_showing) {
        bool showing = true;
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(it->name.c_str(), &showing)) {
            if (it->drawUI >= 0) {
                lua_rawgeti(lua_mainthread, LUA_REGISTRYINDEX, it->drawUI);
                if (lua_isfunction(lua_mainthread, -1)) {
                    lua_callfunction(lua_mainthread, this);
                }
                lua_settop(lua_mainthread, 0);
            }
        }
        checkfocus();
        ImGui::End();
        if (!showing) {
            rmlist.push_back(it);
        }
    }
    for (auto it : rmlist) {
        modules_showing.erase(it);
    }

    module_nodeEditor();
}
void renderContext::module_loop() {
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
void renderContext::shutdownModules() {
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

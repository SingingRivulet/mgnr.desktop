#include "lua_imgui.hpp"
#include "mgenner.h"
void mgenner::loadConfig() {
    bzero(module_importFilePath, sizeof(module_importFilePath));

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

    ImNodes::CreateContext();
    ImNodes::SetNodeGridSpacePos(1, ImVec2(200.0f, 200.0f));
}
void mgenner::module_menu() {
    if (ImGui::MenuItem("导入文件")) {
        module_importFile = true;
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

    module_importWindow();

    /*
    ImGui::Begin("控制");

    auto wpos_min = ImGui::GetWindowPos();
    auto wpos_max = ImVec2(
        wpos_min.x + ImGui::GetWindowWidth(),
        wpos_min.y + ImGui::GetWindowHeight());
    checkfocus();
    if (ImGui::IsMouseHoveringRect(wpos_min, wpos_max)) {
        focusCanvas = false;
    }

    ImNodes::BeginNodeEditor();
    ImNodes::BeginNode(1);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("播放输出");
    ImNodes::EndNodeTitleBar();

    //ImNodes::BeginInputAttribute(2);
    //ImGui::Text("input");
    //ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(3);
    ImGui::Indent(40);
    ImGui::Text("输出");
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
    ImNodes::EndNodeEditor();

    ImGui::End();
    */
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

void mgenner::module_importWindow() {
    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
    if (module_importFile && ImGui::Begin("导入文件", &module_importFile)) {
        auto wpos_min = ImGui::GetWindowPos();
        auto wpos_max = ImVec2(
            wpos_min.x + ImGui::GetWindowWidth(),
            wpos_min.y + ImGui::GetWindowHeight());
        checkfocus();
        if (ImGui::IsMouseHoveringRect(wpos_min, wpos_max)) {
            focusCanvas = false;
        }

        if (fileDialog_importMidi.HasSelected()) {
            snprintf(module_importFilePath,
                     sizeof(module_importFilePath), "%s",
                     fileDialog_importMidi.GetSelected().c_str());
            fileDialog_importMidi.ClearSelected();
        }
        ImNodes::BeginNodeEditor();

        //添加节点
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
            ImNodes::IsEditorHovered() && ImGui::IsKeyReleased(SDL_SCANCODE_A)) {
            const int node_id = module_currentId;
            module_currentId += (1 << 8);
            ImNodes::SetNodeScreenSpacePos(node_id, ImGui::GetMousePos());
            ImNodes::SnapNodeToGrid(node_id);
            module_node n;
            module_nodes[node_id] = n;
        }

        //主节点
        ImNodes::BeginNode(0);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("加载文件");
        ImNodes::EndNodeTitleBar();
        ImNodes::BeginStaticAttribute(1);
        if (module_importFilePath[0] != '\0') {
            auto file = strrchr(module_importFilePath, '/');
            if (file == nullptr) {
                file = module_importFilePath;
            } else {
                ++file;
            }
            ImGui::TextWrapped("%s", file);
        }
        if (ImGui::Button("选择文件")) {
            fileDialog_importMidi.Open();
        }
        ImNodes::EndStaticAttribute();
        ImNodes::BeginOutputAttribute(2);
        ImGui::Indent(40);
        ImGui::Text("输出");
        ImNodes::EndOutputAttribute();
        ImNodes::EndNode();

        for (auto& node : module_nodes) {
            ImNodes::BeginNode(node.first);

            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("node");
            ImNodes::EndNodeTitleBar();

            ImNodes::BeginInputAttribute(node.first + 1);
            ImGui::TextUnformatted("input");
            ImNodes::EndInputAttribute();

            ImNodes::BeginOutputAttribute(node.first + 2);
            const float text_width = ImGui::CalcTextSize("output").x;
            ImGui::Indent(120.f + ImGui::CalcTextSize("value").x - text_width);
            ImGui::TextUnformatted("output");
            ImNodes::EndOutputAttribute();

            ImNodes::EndNode();
        }

        for (auto& link : module_links) {
            ImNodes::Link(link.first, link.second.start_attr, link.second.end_attr);
        }

        ImNodes::EndNodeEditor();
        {  //创建节点
            module_link link;
            if (ImNodes::IsLinkCreated(&link.start_attr, &link.end_attr)) {
                const int node_id = module_currentId;
                module_currentId += (1 << 8);
                module_links[node_id] = link;
            }
        }

        {  //删除节点
            int link_id;
            if (ImNodes::IsLinkDestroyed(&link_id)) {
                module_links.erase(link_id);
            }
        }

        ImGui::End();
    }
}

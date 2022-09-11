#include "mgenner.h"
#include "sysmodule/fileselector.h"
#include "sysmodule/loadToWindow.h"
#include "sysmodule/midiloader.h"
#include "sysmodule/print.h"

void renderContext::module_nodeEditor() {
    if (module_showNodeEditor) {
        ImGui::SetNextWindowPos(ImVec2(9, 55), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(847, 560), ImGuiCond_FirstUseEver);
        vscript.draw(&module_showNodeEditor);
        if (vscript.focused) {
            focusCanvas = false;
        }
        ImGui::SetNextWindowPos(ImVec2(847 + 9, 55), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(387, 560), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("节点输出")) {
            ImGui::LogToClipboard();
            for (auto& it : scriptConsole) {
                ImGui::TextWrapped("%s", it.c_str());
            }
            ImGui::LogFinish();
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
            if (ImGui::BeginPopupContextWindow("控制台操作")) {
                if (ImGui::MenuItem("清空输出")) {
                    scriptConsole.clear();
                }
                ImGui::EndPopup();
            }
        }
        checkfocus();
        ImGui::End();
    }
}

void renderContext::vscript_init() {
    vscript.global = this;
    ImNodes::CreateContext();
    ImNodes::SetNodeGridSpacePos(1, ImVec2(200.0f, 200.0f));
    ///////////////////////////////////////////////////////////////////////////
    //创建初始节点
    //节点类

#define loadModule(name)                                             \
    [this](vscript_t* s) {                                           \
        std::unique_ptr<mgnr::vscript::node> n(new name(this));      \
        auto p = s->addNode(std::move(n));                           \
        ImNodes::SetNodeScreenSpacePos(p->id, ImGui::GetMousePos()); \
        ImNodes::SnapNodeToGrid(p->id);                              \
        return p;                                                    \
    }
#define addModule(dir, title, name)                           \
    vscript.scriptClass[dir].push_back(                       \
        std::tuple<                                           \
            std::string,                                      \
            std::function<mgnr::vscript::node*(vscript_t*)>>( \
            title, loadModule(name)));

    addModule("输入输出", "文本输出", node_print);
    addModule("输入输出", "文件选择", node_getFile);
    addModule("输入输出", "midi加载", node_midiLoader);
    addModule("输入输出", "导入midi至当前窗口", node_loadToWindow);
}

void renderContext::vscript_t::addNodeAt(mgnr::vscript::port_output* p) {
    addNodeMode = true;
    addNodeAtPort = p;
    addNodeAtPort_window_pos = ImVec2(global->mouse_x, global->mouse_y);
}
void renderContext::vscript_t::onAddNode() {
    if (ImGui::BeginPopup("添加节点")) {
        global->checkfocus();
        for (auto& menu : scriptClass) {
            if (ImGui::BeginMenu(menu.first.c_str())) {
                for (auto& it : menu.second) {
                    if (ImGui::MenuItem(std::get<0>(it).c_str())) {
                        auto p = std::get<1>(it)(this);
                        if (p && p->input.size() == 1) {
                            addLink(addNodeAtPort, p->input.at(0).get());
                        }
                    }
                }
                ImGui::EndMenu();
            }
        }

        if (ImGui::BeginMenu("其他节点")) {
            for (auto& it : scriptClass_other) {
                if (ImGui::MenuItem(std::get<0>(it).c_str())) {
                    auto p = std::get<1>(it)(this);
                    if (p && p->input.size() == 1) {
                        addLink(addNodeAtPort, p->input.at(0).get());
                    }
                }
            }
            ImGui::EndMenu();
        }
        ImGui::End();
    }

    if (addNodeMode) {
        ImGui::OpenPopup("添加节点");
        addNodeMode = false;
    }

    if (ImGui::BeginPopupContextWindow()) {
        global->checkfocus();
        if (!node_selected.empty()) {
            if (ImGui::MenuItem("删除节点")) {
                for (auto it : node_selected) {
                    removeList.push_back(it);
                }
            }
        }
        if (!link_selected.empty()) {
            if (ImGui::MenuItem("删除连线")) {
                for (auto it : link_selected) {
                    delLink(it);
                }
                link_selected.clear();
            }
        }
        if (ImGui::MenuItem("清除状态")) {
            reset = true;
        }
        if (ImGui::BeginMenu("添加节点")) {
            for (auto& menu : scriptClass) {
                if (ImGui::BeginMenu(menu.first.c_str())) {
                    for (auto& it : menu.second) {
                        if (ImGui::MenuItem(std::get<0>(it).c_str())) {
                            std::get<1>(it)(this);
                        }
                    }
                    ImGui::EndMenu();
                }
            }

            if (ImGui::BeginMenu("其他节点")) {
                for (auto& it : scriptClass_other) {
                    if (ImGui::MenuItem(std::get<0>(it).c_str())) {
                        std::get<1>(it)(this);
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
}

renderContext::vscript_t::vscript_t() {
    title = "节点控制台";
}
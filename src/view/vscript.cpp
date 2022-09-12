#include "mgenner.h"
#include "sysmodule/fileselector.h"
#include "sysmodule/loadToWindow.h"
#include "sysmodule/midiloader.h"
#include "sysmodule/print.h"
#include "sysmodule/wavloader.h"

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

void renderContext::addVClass(std::shared_ptr<renderContext::vclass_t> p) {
    for (auto& type : p->input_types) {
        vscript.scriptClass_type[type].push_back(p);
    }
    if (p->menu.empty()) {
        vscript.scriptClass_other.push_back(std::move(p));
    } else {
        vscript.scriptClass[p->menu].push_back(std::move(p));
    }
}

void renderContext::vscript_t::buildViewClass(const std::string& type) {
    scriptClass_view.clear();
    {
        auto t = scriptClass_type.find(std::string(""));
        if (t != scriptClass_type.end()) {
            for (auto it : t->second) {
                scriptClass_view.push_back(it);
            }
        }
    }
    if (!type.empty()) {
        auto t = scriptClass_type.find(type);
        if (t != scriptClass_type.end()) {
            for (auto it : t->second) {
                scriptClass_view.push_back(it);
            }
        }
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
#define addModule(dir, title_i, name, input)                  \
    {                                                         \
        auto p = std::make_shared<renderContext::vclass_t>(); \
        p->callback = loadModule(name);                       \
        p->title = title_i;                                   \
        p->menu = dir;                                        \
        p->input_types = std::set<std::string>(input);        \
        addVClass(p);                                         \
    }
    addModule("输入输出", "文本输出", node_print, {std::string("字符串")});
    addModule("输入输出", "文件选择", node_getFile, {});
    addModule("输入输出", "导入midi至当前窗口", node_loadToWindow, {std::string("midi数据")});
    addModule("文件处理", "midi加载", node_midiLoader, {std::string("字符串")});
    addModule("文件处理", "wav加载", node_wavLoader, {std::string("字符串")});
}

void renderContext::vscript_t::addNodeAt(mgnr::vscript::port_output* p) {
    addNodeMode = true;
    buildViewClass(p->type);
    addNodeAtPort = p;
    addNodeAtPort_window_pos = ImVec2(global->mouse_x, global->mouse_y);
}
void renderContext::vscript_t::onAddNode() {
    if (ImGui::BeginPopup("添加节点")) {
        global->checkfocus();
        for (auto& it : scriptClass_view) {
            if (ImGui::MenuItem(it->title.c_str())) {
                auto p = it->callback(this);
                if (p && p->input.size() == 1) {
                    addLink(addNodeAtPort, p->input.at(0).get());
                }
            }
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
                        if (ImGui::MenuItem(it->title.c_str())) {
                            it->callback(this);
                        }
                    }
                    ImGui::EndMenu();
                }
            }

            if (!scriptClass_other.empty()) {
                if (ImGui::BeginMenu("其他节点")) {
                    for (auto& it : scriptClass_other) {
                        if (ImGui::MenuItem(it->title.c_str())) {
                            it->callback(this);
                        }
                    }
                    ImGui::EndMenu();
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
}

renderContext::vscript_t::vscript_t() {
    title = "节点控制台";
}
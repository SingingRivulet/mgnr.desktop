#include "mgenner.h"

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
//文件选择节点
struct node_getFile : public mgnr::vscript::node_ui {
    renderContext* global;
    char module_importFilePath[512];
    node_getFile(renderContext* global) {
        this->global = global;
        bzero(module_importFilePath, sizeof(module_importFilePath));
        this->name = "加载文件";
        std::unique_ptr<mgnr::vscript::port_output> out0(new mgnr::vscript::port_output);
        out0->name = "文件路径";
        out0->type = "字符串";
        this->output.push_back(std::move(out0));
        this->removeable = false;
    }
    void exec() override {
        global->scriptConsole.push_back(
            std::string("加载文件:") + module_importFilePath);
        auto p = std::make_shared<mgnr::vscript::value>();
        p->data = std::string(module_importFilePath);
        this->output[0]->send(p);
    }
    void draw() override {
        if (global->fileDialog_importMidi.HasSelected()) {
            snprintf(module_importFilePath,
                     sizeof(module_importFilePath), "%s",
                     global->fileDialog_importMidi.GetSelected().c_str());
            global->fileDialog_importMidi.ClearSelected();
        }

        ImNodes::BeginStaticAttribute(staticAttributeId);
        if (module_importFilePath[0] != '\0') {
            auto file = editWindow::getFileName(module_importFilePath);
            ImGui::TextWrapped("%s", file);
        }
        if (ImGui::Button("选择文件")) {
            global->fileDialog_importMidi.Open();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("设置该节点的输出值");
        }
        if (ImGui::Button("执行")) {
            parent->exec(this);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("向右边端点所连接的节点发送数据");
        }
        ImNodes::EndStaticAttribute();
    }
    ~node_getFile() override {}
};
//输出节点
struct node_print : public mgnr::vscript::node_ui {
    renderContext* global;
    node_print(renderContext* global) {
        this->global = global;
        this->name = "输出";
        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "输出字符串";
        in0->type = "字符串";
        this->input.push_back(std::move(in0));
    }
    void exec() override {
        for (auto& it : input[0]->data) {
            try {
                global->scriptConsole.push_back(std::any_cast<std::string>(it->data).c_str());
            } catch (...) {
                global->scriptConsole.push_back("节点输出失败");
            }
        }
        input[0]->data.clear();
    }
    ~node_print() override {}
};

void renderContext::vscript_init() {
    vscript.global = this;
    ImNodes::CreateContext();
    ImNodes::SetNodeGridSpacePos(1, ImVec2(200.0f, 200.0f));
    ///////////////////////////////////////////////////////////////////////////
    //创建初始节点
    std::unique_ptr<mgnr::vscript::node> node_getFile_n(new node_getFile(this));
    vscript.addNode(std::move(node_getFile_n));
    //节点类
    vscript.scriptClass["输入输出"].push_back(
        std::tuple<
            std::string,
            std::function<mgnr::vscript::node*(vscript_t*)>>(
            "控制台输出节点",
            [this](vscript_t* s) {
                std::unique_ptr<mgnr::vscript::node> n(new node_print(this));
                auto p = s->addNode(std::move(n));
                ImNodes::SetNodeScreenSpacePos(p->id, ImGui::GetMousePos());
                ImNodes::SnapNodeToGrid(p->id);
                return p;
            }));
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
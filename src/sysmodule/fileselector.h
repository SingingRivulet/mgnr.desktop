#pragma once
#include "view/mgenner.h"
//文件选择节点
struct node_getFile : public mgnr::vscript::node_ui {
    renderContext* global;
    char module_importFilePath[512];
    ImGui::FileBrowser fileDialog_importMidi;
    node_getFile(renderContext* global) {
        this->global = global;
        bzero(module_importFilePath, sizeof(module_importFilePath));
        this->name = "加载文件";
        std::unique_ptr<mgnr::vscript::port_output> out0(new mgnr::vscript::port_output);
        out0->name = "文件路径";
        out0->type = "字符串";
        this->output.push_back(std::move(out0));
    }
    void exec() override {
        global->scriptConsole.push_back(
            std::string("加载文件:") + module_importFilePath);
        auto p = std::make_shared<mgnr::vscript::value_string>();
        p->data = std::string(module_importFilePath);
        this->output[0]->send(p);
    }
    void draw() override {
        if (fileDialog_importMidi.HasSelected()) {
            snprintf(module_importFilePath,
                     sizeof(module_importFilePath), "%s",
                     fileDialog_importMidi.GetSelected().c_str());
            fileDialog_importMidi.ClearSelected();
        }

        ImNodes::BeginStaticAttribute(staticAttributeId);
        if (module_importFilePath[0] != '\0') {
            auto file = editWindow::getFileName(module_importFilePath);
            ImGui::TextWrapped("%s", file);
        }
        if (ImGui::Button("选择文件")) {
            fileDialog_importMidi.Open();
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
        fileDialog_importMidi.Display();
        if (!fileDialog_importMidi.focusCanvas) {
            dynamic_cast<renderContext::vscript_t*>(parent)->focused = true;
        }
    }
    ~node_getFile() override {}
};
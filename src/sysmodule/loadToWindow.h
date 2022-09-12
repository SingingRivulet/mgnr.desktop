#pragma once
#include "types/midifile.h"
#include "view/mgenner.h"
struct node_loadToWindow : public mgnr::vscript::node_ui {
    renderContext* global;
    node_loadToWindow(renderContext* global) {
        this->global = global;
        this->name = "导入midi至当前窗口";
        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "midi数据";
        in0->type = "midi数据";
        this->input.push_back(std::move(in0));
    }
    void exec() override {
        for (auto& it : input[0]->data) {
            try {
                auto midi = (std::dynamic_pointer_cast<midifile_t>(it));
                if (midi != nullptr) {
                    for (auto it : midi->notes) {
                        global->drawing->addNote(it->begin, it->tone, it->duration, it->volume, it->info);
                    }
                } else {
                    global->scriptConsole.push_back("加载midi失败");
                }
            } catch (...) {
                global->scriptConsole.push_back("加载midi失败");
            }
        }
        input[0]->data.clear();
    }
    ~node_loadToWindow() override {}
};
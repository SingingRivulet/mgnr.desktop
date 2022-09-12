#pragma once
#include "view/mgenner.h"
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
                auto p = std::dynamic_pointer_cast<mgnr::vscript::value_string>(it);
                if (p != nullptr) {
                    global->scriptConsole.push_back(p->data);
                } else {
                    global->scriptConsole.push_back("类型不匹配");
                }
            } catch (...) {
                global->scriptConsole.push_back("节点输出失败");
            }
        }
        input[0]->data.clear();
    }
    ~node_print() override {}
};

#pragma once
#include "types/wavfile.h"
#include "view/mgenner.h"

//wav节点
struct node_wavLoader : public mgnr::vscript::node_ui {
    renderContext* global;
    node_wavLoader(renderContext* global) {
        this->global = global;
        this->name = "加载wav文件";

        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "文件路径";
        in0->type = "字符串";
        this->input.push_back(std::move(in0));

        std::unique_ptr<mgnr::vscript::port_output> out0(new mgnr::vscript::port_output);
        out0->name = "wav数据";
        out0->type = "wav数据";
        this->output.push_back(std::move(out0));
    }
    void exec() override {
        for (auto& it : input[0]->data) {
            try {
                auto path = std::dynamic_pointer_cast<mgnr::vscript::value_string>(it);
                if (path != nullptr) {
                    auto p = std::make_shared<wav_input_t>(path->data);
                    this->output[0]->send(p);
                } else {
                    global->scriptConsole.push_back("加载wav文件：获取路径失败");
                }
            } catch (std::bad_cast&) {
                global->scriptConsole.push_back("加载wav文件：类型转换错误");
            } catch (std::runtime_error& e) {
                global->scriptConsole.push_back(std::string("加载wav文件：") + e.what());
            } catch (...) {
                global->scriptConsole.push_back("加载wav文件：未知错误");
            }
        }
        input[0]->data.clear();
    }
    ~node_wavLoader() override {}
};
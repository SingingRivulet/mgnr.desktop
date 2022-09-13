#pragma once
#include "types/wavfile.h"
#include "view/mgenner.h"
struct node_wavFrameSampler : public mgnr::vscript::node_ui {
    renderContext* global;
    node_wavFrameSampler(renderContext* global) {
        this->global = global;
        this->name = "wav帧采样";

        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "wav数据";
        in0->type = "wav数据";
        this->input.push_back(std::move(in0));

        std::unique_ptr<mgnr::vscript::port_output> out0(new mgnr::vscript::port_output);
        out0->name = "wav帧采样器";
        out0->type = "wav帧采样器";
        this->output.push_back(std::move(out0));
    }
    void exec() override {
        for (auto& it : input[0]->data) {
            try {
                auto wav = std::dynamic_pointer_cast<wav_input_t>(it);
                if (wav != nullptr) {
                    auto p = std::make_shared<frameSampler_t>(wav);
                    this->output[0]->send(p);
                } else {
                    global->scriptConsole.push_back("加载wav文件：文件无法打开");
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
    ~node_wavFrameSampler() override {}
};
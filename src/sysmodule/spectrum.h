#pragma once
#include "types/wavfile.h"
#include "view/mgenner.h"
struct node_spectrum : public mgnr::vscript::node_ui {
    renderContext* global;
    node_spectrum(renderContext* global) {
        this->global = global;
        this->name = "频谱生成";

        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "波形帧数据流";
        in0->type = "波形帧数据流";
        this->input.push_back(std::move(in0));

        std::unique_ptr<mgnr::vscript::port_output> out0(new mgnr::vscript::port_output);
        out0->name = "频谱";
        out0->type = "频谱";
        this->output.push_back(std::move(out0));
    }
    void exec() override {
        for (auto& it : input[0]->data) {
            try {
                auto sampler = std::dynamic_pointer_cast<frameStream_t>(it);
                if (sampler != nullptr) {
                    auto p = std::make_shared<spectrumBuilder_t>(sampler);
                    this->output[0]->send(p);
                } else {
                    errors.push_back("无法进行采样");
                }
            } catch (std::bad_cast&) {
                errors.push_back("生成频谱：类型转换错误");
            } catch (std::runtime_error& e) {
                errors.push_back(std::string("生成频谱：") + e.what());
            } catch (...) {
                errors.push_back("生成频谱：未知错误");
            }
        }
        input[0]->data.clear();
    }
    ~node_spectrum() override {}
};
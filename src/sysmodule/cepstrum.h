#pragma once
#include "types/wavfile.h"
#include "view/mgenner.h"
struct node_cepstrum : public mgnr::vscript::node_ui {
    renderContext* global;
    node_cepstrum(renderContext* global) {
        this->global = global;
        this->name = "频谱生成";

        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "频谱";
        in0->type = "频谱";
        this->input.push_back(std::move(in0));

        std::unique_ptr<mgnr::vscript::port_output> out0(new mgnr::vscript::port_output);
        out0->name = "倒频谱";
        out0->type = "倒频谱";
        this->output.push_back(std::move(out0));
    }
    void exec() override {
        for (auto& it : input[0]->data) {
            try {
                auto spec = std::dynamic_pointer_cast<spectrumBuilder_t>(it);
                if (spec != nullptr) {
                    auto p = std::make_shared<cepstrumBuilder_t>(spec);
                    this->output[0]->send(p);
                } else {
                    errors.push_back("无法获取频谱");
                }
            } catch (std::bad_cast&) {
                errors.push_back("生成倒频谱：类型转换错误");
            } catch (std::runtime_error& e) {
                errors.push_back(std::string("生成倒频谱：") + e.what());
            } catch (...) {
                errors.push_back("生成倒频谱：未知错误");
            }
        }
        input[0]->data.clear();
    }
    ~node_cepstrum() override {}
};
#pragma once
#include "types/wavfile.h"
#include "view/mgenner.h"
struct node_spectrumViewer : public mgnr::vscript::node_ui {
    renderContext* global;
    int overlap = 2;
    node_spectrumViewer(renderContext* global) {
        this->global = global;
        this->name = "频谱查看器";

        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "频谱数据";
        in0->type = "";
        this->input.push_back(std::move(in0));
    }
    void draw() override {
        //ImNodes::BeginStaticAttribute(staticAttributeId);
        //ImGui::SetNextItemWidth(100);
        //ImNodes::EndStaticAttribute();
        mgnr::vscript::node_ui::draw();
    }
    void exec() override {
        for (auto& it : input[0]->data) {
            try {
                auto ceps = std::dynamic_pointer_cast<cepstrumBuilder_t>(it);
                if (ceps != nullptr) {
                    ceps->read([&](const wav_frame_t& w) {
                    });
                } else {
                    auto spec = std::dynamic_pointer_cast<spectrumBuilder_t>(it);
                    if (spec != nullptr) {
                        spec->read([&](const spectrum_t& w) {
                        });
                    } else {
                        errors.push_back("获取频谱：数据无法识别");
                    }
                }
            } catch (std::bad_cast&) {
                errors.push_back("获取频谱：类型转换错误");
            } catch (std::runtime_error& e) {
                errors.push_back(std::string("获取频谱：") + e.what());
            } catch (...) {
                errors.push_back("获取频谱：未知错误");
            }
        }
        input[0]->data.clear();
    }
    ~node_spectrumViewer() override {}
};
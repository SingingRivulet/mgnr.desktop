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
                    errors.push_back("加载wav文件：获取路径失败");
                }
            } catch (std::bad_cast&) {
                errors.push_back("加载wav文件：类型转换错误");
            } catch (std::runtime_error& e) {
                errors.push_back(std::string("加载wav文件：") + e.what());
            } catch (...) {
                errors.push_back("加载wav文件：未知错误");
            }
        }
        input[0]->data.clear();
    }
    ~node_wavLoader() override {}
};

struct node_wavExport : public mgnr::vscript::node_ui {
    renderContext* global;
    node_wavExport(renderContext* global) {
        this->global = global;
        this->name = "加载wav文件";

        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "文件路径";
        in0->type = "字符串";
        this->input.push_back(std::move(in0));

        std::unique_ptr<mgnr::vscript::port_input> in1(new mgnr::vscript::port_input);
        in1->name = "wav数据";
        in1->type = "wav数据";
        this->input.push_back(std::move(in1));
    }
    void exec() override {
        while (!input[0]->data.empty() && !input[1]->data.empty()) {
            auto data_path = input[0]->data.front();
            auto data_wave = input[1]->data.front();
            try {
                auto path = std::dynamic_pointer_cast<mgnr::vscript::value_string>(data_path);
                auto wave = std::dynamic_pointer_cast<wav_input_t>(data_wave);
                if (path != nullptr && wave != nullptr) {
                    WavOutFile out(path->data.c_str(),
                                   wave->getSampleRate(),
                                   wave->getNumBits(),
                                   wave->getNumChannel());
                    wave->read(
                        [&](float* fbuffer, int size) {
                            out.write(fbuffer, size);
                        },
                        false);
                } else {
                    if (path == nullptr) {
                        errors.push_back("保存wav文件：路径获取错误");
                    }
                    if (wave == nullptr) {
                        errors.push_back("保存wav文件：数据获取错误");
                    }
                }
            } catch (std::bad_cast&) {
                errors.push_back("保存wav文件：类型转换错误");
            } catch (std::runtime_error& e) {
                errors.push_back(std::string("保存wav文件：") + e.what());
            } catch (...) {
                errors.push_back("保存wav文件：未知错误");
            }
            input[0]->data.pop_front();
            input[1]->data.pop_front();
        }
    }
    ~node_wavExport() override {}
};
#pragma once
#include "types/wavfile.h"
#include "view/mgenner.h"
struct node_wavFrameSampler : public mgnr::vscript::node_ui {
    renderContext* global;
    int overlap = 2;
    node_wavFrameSampler(renderContext* global) {
        this->global = global;
        this->name = "wav帧采样";

        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "wav数据";
        in0->type = "wav数据";
        this->input.push_back(std::move(in0));

        std::unique_ptr<mgnr::vscript::port_output> out0(new mgnr::vscript::port_output);
        out0->name = "波形帧数据流";
        out0->type = "波形帧数据流";
        this->output.push_back(std::move(out0));
    }
    void draw() override {
        ImNodes::BeginStaticAttribute(staticAttributeId);
        ImGui::SetNextItemWidth(100);
        if (ImGui::InputInt("重叠", &overlap)) {
            if (overlap > 8) {
                overlap = 8;
            }
            if (overlap < 1) {
                overlap = 1;
            }
        }
        ImNodes::EndStaticAttribute();
        mgnr::vscript::node_ui::draw();
    }
    void exec() override {
        for (auto& it : input[0]->data) {
            try {
                auto wav = std::dynamic_pointer_cast<wav_input_t>(it);
                if (wav != nullptr) {
                    auto p = std::make_shared<frameSampler_t>(wav);
                    p->overlap = this->overlap;
                    this->output[0]->send(p);
                } else {
                    errors.push_back("加载wav文件：文件无法打开");
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
    ~node_wavFrameSampler() override {}
};

struct EnumDesc {
    sinrivUtils::waveWindow::SoundWindowType Value;
    const char* Name;
};
static const EnumDesc windowPolicies[] = {
    {sinrivUtils::waveWindow::SOUND_WINDOW_HANNING, "HANNING"},
    {sinrivUtils::waveWindow::SOUND_WINDOW_HAMMING, "HAMMING"},
    {sinrivUtils::waveWindow::SOUND_WINDOW_TRIANGULAR, "三角形"},
    {sinrivUtils::waveWindow::SOUND_WINDOW_GAUSS, "GAUSS"},
    {sinrivUtils::waveWindow::SOUND_WINDOW_BLACKMAN_HARRIS, "BLACKMAN_HARRIS"},
    {sinrivUtils::waveWindow::SOUND_WINDOW_FLAT, "平坦"},
    {sinrivUtils::waveWindow::SOUND_WINDOW_RANDOM, "随机"},
    {sinrivUtils::waveWindow::SOUND_WINDOW_ALMOST_FLAT, "几乎平坦"},
};
struct node_wavFrameWindow : public mgnr::vscript::node_ui {
    renderContext* global;
    node_wavFrameWindow(renderContext* global) {
        this->global = global;
        this->name = "波形数据帧加窗";

        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "波形帧数据流";
        in0->type = "波形帧数据流";
        this->input.push_back(std::move(in0));

        std::unique_ptr<mgnr::vscript::port_output> out0(new mgnr::vscript::port_output);
        out0->name = "波形帧数据流";
        out0->type = "波形帧数据流";
        this->output.push_back(std::move(out0));
    }
    int windowIndex = 0;
    void draw() override {
        ImNodes::BeginStaticAttribute(staticAttributeId);
        ImGui::SetNextItemWidth(100);
        if (ImGui::BeginCombo("窗函数", windowPolicies[windowIndex].Name)) {
            for (int n = 0; n < IM_ARRAYSIZE(windowPolicies); n++) {
                if (ImGui::Selectable(windowPolicies[n].Name, windowIndex == n)) {
                    windowIndex = n;
                }
            }
            ImGui::EndCombo();
        }
        ImNodes::EndStaticAttribute();
        mgnr::vscript::node_ui::draw();
    }
    void exec() override {
        for (auto& it : input[0]->data) {
            try {
                auto in = std::dynamic_pointer_cast<frameSampler_t>(it);
                if (in != nullptr) {
                    auto p = std::make_shared<frameWindow_t>(in);
                    p->windowType = windowPolicies[windowIndex].Value;
                    this->output[0]->send(p);
                } else {
                    errors.push_back("加载波形数据：类型不匹配");
                }
            } catch (std::bad_cast&) {
                errors.push_back("加载波形数据：类型转换错误");
            } catch (std::runtime_error& e) {
                errors.push_back(std::string("加载波形数据：") + e.what());
            } catch (...) {
                errors.push_back("加载波形数据：未知错误");
            }
        }
        input[0]->data.clear();
    }
    ~node_wavFrameWindow() override {}
};
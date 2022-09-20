#pragma once
#include "spectrum/renderer.h"
#include "types/wavfile.h"
#include "view/mgenner.h"

struct node_spectrumViewer : public mgnr::vscript::node_ui {
    renderContext* global;
    mgnr::spectrum::renderer renderer;
    node_spectrumViewer(renderContext* global) {
        renderer.sdlrenderer = global->renderer;

        this->global = global;
        this->name = "频谱查看器";

        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "频谱数据";
        in0->type = "";
        this->input.push_back(std::move(in0));
    }
    bool showSpec = false;
    void draw() override {
        if (showSpec) {
            if (ImGui::Begin("频谱显示", &showSpec)) {
                global->checkfocus();
                ImGuiIO& io = ImGui::GetIO();
                ImGui::SetWindowFocus();
                auto pos = ImGui::GetWindowPos();
                int w = ImGui::GetWindowWidth();
                int h = ImGui::GetWindowHeight();
                if (w != renderer.viewPort.windowSize.x ||
                    h != renderer.viewPort.windowSize.y) {
                    //printf("spectrum:width=%d,height=%d\n", w, h);
                    renderer.viewPort.windowSize.x = w;
                    renderer.viewPort.windowSize.y = h;
                    renderer.needUpdate = true;
                }
                renderer.render();
                if (ImGui::IsWindowFocused()) {
                    int wheel = io.MouseWheel;
                    if (wheel > 0) {
                        renderer.viewPort.scale *= 1.1;
                        renderer.needUpdate = true;
                    } else if (wheel < 0) {
                        renderer.viewPort.scale /= 1.1;
                        if (renderer.viewPort.scale < 1) {
                            renderer.viewPort.scale = 1;
                        }
                        renderer.needUpdate = true;
                    }
                    if (ImGui::IsMouseDown(2)) {
                        renderer.viewPort.setLookAt(
                            mgnr::spectrum::vec2<int>(
                                renderer.viewPort.lookAtBegin.x - io.MouseDelta.x * renderer.viewPort.scale,
                                renderer.viewPort.lookAtBegin.y - io.MouseDelta.y * renderer.viewPort.scale));
                        renderer.needUpdate = true;
                    }
                }
            }
            ImGui::End();
        }
        ImNodes::BeginStaticAttribute(staticAttributeId);
        if (ImGui::Button("查看")) {
            renderer.layout_draw = nullptr;
            renderer.layout_show = nullptr;
            renderer.needUpdate = true;
            renderer.viewPort.updateLookAt();
            showSpec = true;
        }
        ImNodes::EndStaticAttribute();
        mgnr::vscript::node_ui::draw();
    }
    void exec() override {
        for (auto& it : input[0]->data) {
            try {
                auto ceps = std::dynamic_pointer_cast<cepstrumBuilder_t>(it);
                if (ceps != nullptr) {
                    renderer.height = 0;
                    renderer.width = 0;
                    renderer.data.clear();
                    ceps->read([&](const wav_frame_t& w) {
                        if (w.channel >= 1) {
                            auto data = w[0];
                            renderer.height = w.size;
                            ++renderer.width;
                            for (int i = 0; i < w.size; ++i) {
                                auto value = data[i];
                                renderer.data.push_back(value);
                            }
                        }
                    });
                    renderer.updateDate();
                    global->scriptConsole.push_back("频谱已更新");
                    break;
                } else {
                    auto spec = std::dynamic_pointer_cast<spectrumBuilder_t>(it);
                    if (spec != nullptr) {
                        renderer.height = 0;
                        renderer.width = 0;
                        renderer.data.clear();
                        spec->read([&](const spectrum_t& w) {
                            if (w.channel >= 1) {
                                auto data = w[0];
                                int len = w.size / 2;
                                renderer.height = len;
                                ++renderer.width;
                                for (int i = 0; i < len; ++i) {
                                    auto value = sqrt(data[i].r * data[i].r + data[i].i * data[i].i);
                                    renderer.data.push_back(value);
                                }
                            }
                        });
                        renderer.updateDate();
                        global->scriptConsole.push_back("频谱已更新");
                        break;
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
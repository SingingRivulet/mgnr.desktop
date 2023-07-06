#pragma once
#include "spectrum/full.h"
#include "spectrum/step.h"
#include "types/wavfile.h"
#include "view/mgenner.h"

struct node_spectrumViewer : public mgnr::vscript::node_ui {
    renderContext* global;
    mgnr::spectrum::fullRenderer fullRenderer;
    mgnr::spectrum::stepRenderer stepRenderer;
    int windowId;
    node_spectrumViewer(renderContext* global) {
        windowId = time(0);
        fullRenderer.sdlrenderer = global->renderer;
        stepRenderer.parent = &fullRenderer;

        this->global = global;
        this->name = "频谱查看器";

        std::unique_ptr<mgnr::vscript::port_input> in0(new mgnr::vscript::port_input);
        in0->name = "频谱数据";
        in0->type = "";
        this->input.push_back(std::move(in0));
    }
    bool showSpec = false;
    int stepCursor = -1;
    int specCursor = -1;
    bool fullSpecWindowMoveable = true;
    bool stepSpecWindowMoveable = true;
    void draw() override {
        char title[128];
        if (showSpec) {
            ImGui::SetNextWindowSize(ImVec2(800, 590), ImGuiCond_FirstUseEver);
            snprintf(title, sizeof(title), "频谱显示##%d", this->windowId);
            int windowFlag = 0;
            if (!fullSpecWindowMoveable) {
                windowFlag |= ImGuiWindowFlags_NoMove;
            }
            if (ImGui::Begin(title, &showSpec, windowFlag)) {
                //ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
                ImGuiIO& io = ImGui::GetIO();
                //ImGui::SetWindowFocus();
                auto pos = ImGui::GetWindowPos();
                int w = ImGui::GetWindowWidth();
                int h = ImGui::GetWindowHeight();
                if (w != fullRenderer.viewPort.windowSize.x ||
                    h != fullRenderer.viewPort.windowSize.y) {
                    //printf("spectrum:width=%d,height=%d\n", w, h);
                    fullRenderer.viewPort.windowSize.x = w;
                    fullRenderer.viewPort.windowSize.y = h;
                    fullRenderer.needUpdate = true;
                }
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImVec2 pend = p0 + ImVec2(w, h);
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
                draw_list->AddRectFilled(p0, pend, ImColor(ImVec4(0.1, 0.1, 0.1, 1.0f)));
                fullRenderer.render();
                if (stepCursor > 0) {
                    static ImVec4 colf = ImVec4(0.0f, 1.0f, 0.4f, 1.0f);
                    const ImU32 col = ImColor(colf);
                    int cursorPos = (stepCursor - fullRenderer.viewPort.lookAtBegin.x) / fullRenderer.viewPort.scale;
                    draw_list->AddLine(p0 + ImVec2(cursorPos, 0), p0 + ImVec2(cursorPos, h), col);
                }
                if (ImGui::IsWindowFocused()) {
                    int wheel = io.MouseWheel;
                    if (wheel > 0) {
                        fullRenderer.viewPort.scale *= 1.1;
                        fullRenderer.needUpdate = true;
                    } else if (wheel < 0) {
                        fullRenderer.viewPort.scale /= 1.1;
                        if (fullRenderer.viewPort.scale < 1) {
                            fullRenderer.viewPort.scale = 1;
                        }
                        fullRenderer.needUpdate = true;
                    }
                    if (ImGui::IsMouseHoveringRect(p0, pend)) {
                        fullSpecWindowMoveable = false;
                        if (ImGui::IsWindowHovered()) {
                            if (ImGui::IsMouseDown(2)) {
                                fullRenderer.viewPort.setLookAt(
                                    mgnr::spectrum::vec2<int>(
                                        fullRenderer.viewPort.lookAtBegin.x - io.MouseDelta.x * fullRenderer.viewPort.scale,
                                        fullRenderer.viewPort.lookAtBegin.y - io.MouseDelta.y * fullRenderer.viewPort.scale));
                                fullRenderer.needUpdate = true;
                            }
                            if (ImGui::IsMouseDown(0)) {
                                auto mpos = ImGui::GetMousePos();
                                auto wpos = mpos - p0;
                                auto spos = wpos.x * fullRenderer.viewPort.scale + fullRenderer.viewPort.lookAtBegin.x;
                                stepCursor = spos;
                                stepRenderer.get(stepCursor);
                            }
                        }
                    } else {
                        fullSpecWindowMoveable = true;
                    }
                }
            }
            ImGui::End();
            if (stepCursor > 0 && stepRenderer.len > 0) {
                int windowFlag = ImGuiWindowFlags_NoFocusOnAppearing;
                if (!stepSpecWindowMoveable) {
                    windowFlag |= ImGuiWindowFlags_NoMove;
                }
                ImGui::SetNextWindowSize(ImVec2(925, 500), ImGuiCond_FirstUseEver);
                snprintf(title, sizeof(title), "频谱显示(时刻)##%d", this->windowId);
                if (ImGui::Begin(title, nullptr, windowFlag)) {
                    //ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
                    ImVec2 p0 = ImGui::GetCursorScreenPos() + ImVec2(20, 20);
                    int w = ImGui::GetWindowWidth() - 40;
                    int h = ImGui::GetWindowHeight() - 60;
                    ImVec2 pend = p0 + ImVec2(w, h);
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddRectFilled(p0, p0 + ImVec2(w, h), ImColor(ImVec4(0, 0, 0.2, 1.0f)));
                    auto col = ImColor(ImVec4(1, 1, 1, 1.0f));

                    ImVec2 last(p0.x, p0.y + h);

                    bool pointMode = ((((float)w) / fullRenderer.height) <= 1);

                    if (pointMode) {
                        for (int i = 0; i < w; ++i) {
                            int id = (i * stepRenderer.len) / w;
                            ImVec2 pos(i, stepRenderer.points[id] * h);
                            pos += p0;
                            draw_list->AddLine(last, pos, col);
                            last = pos;
                        }
                    } else {
                        for (int i = 0; i < stepRenderer.len; ++i) {
                            ImVec2 pos((i * w) / stepRenderer.len, stepRenderer.points[i] * h);
                            pos += p0;
                            draw_list->AddLine(last, pos, col);
                            last = pos;
                        }
                    }

                    if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(p0, pend)) {
                        stepSpecWindowMoveable = false;
                        if (ImGui::IsMouseDown(0)) {
                            auto mpos = ImGui::GetMousePos();
                            auto wpos = (mpos - p0).x;
                            if (wpos >= 0 && wpos < w) {
                                specCursor = wpos * stepRenderer.len / w;
                            }
                        }
                    } else {
                        stepSpecWindowMoveable = true;
                    }
                    char buf[256];
                    snprintf(buf, sizeof(buf), "帧号：%d", stepCursor);
                    draw_list->AddText(ImVec2(0, 0) + p0, col, buf);
                    if (specCursor >= 0) {
                        snprintf(buf, sizeof(buf), "位置：%d", specCursor);
                        draw_list->AddText(ImVec2(0, 20) + p0, col, buf);
                        ImVec2 pos0((specCursor * w) / stepRenderer.len, 0);
                        ImVec2 pos1((specCursor * w) / stepRenderer.len, h);
                        draw_list->AddLine(pos0 + p0, pos1 + p0, col);
                        try {
                            snprintf(buf, sizeof(buf), "数值：%f",
                                     fullRenderer.data_length.at(stepCursor).at(specCursor));
                            draw_list->AddText(ImVec2(0, 40) + p0, col, buf);

                            if (!fullRenderer.data_phase.empty()) {
                                auto phase = fullRenderer.data_phase.at(stepCursor).at(specCursor);
                                int deg = phase * 180 / M_PI;
                                snprintf(buf, sizeof(buf), "相位：%d", deg);
                                draw_list->AddText(ImVec2(0, 60) + p0, col, buf);
                                /*
                                //测试
                                int btime = 256 * stepCursor;  //当前时间（单位：点）
                                //波的公式：sin(f*fftsize*(t+btime))
                                int bphase = (btime * specCursor) % 512;
                                snprintf(buf, sizeof(buf), "初相：%d", bphase);
                                draw_list->AddText(ImVec2(0, 80) + p0, col, buf);
                                */
                            }
                        } catch (...) {
                        }
                    }
                }
                ImGui::End();
            }
        }
        ImNodes::BeginStaticAttribute(staticAttributeId);
        if (ImGui::Button("查看")) {
            fullRenderer.layout_draw = nullptr;
            fullRenderer.layout_show = nullptr;
            fullRenderer.needUpdate = true;
            fullRenderer.viewPort.updateLookAt();
            showSpec = true;
        }
        ImNodes::EndStaticAttribute();
        mgnr::vscript::node_ui::draw();
    }
    void exec() override {
        showSpec = false;
        for (auto& it : input[0]->data) {
            try {
                auto ceps = std::dynamic_pointer_cast<cepstrumBuilder_t>(it);
                if (ceps != nullptr) {
                    fullRenderer.height = 0;
                    fullRenderer.width = 0;
                    fullRenderer.data_length.clear();
                    fullRenderer.data_phase.clear();
                    fullRenderer.minFreq = 1;
                    fullRenderer.maxFreq = 1;
                    int numFrame = 0;
                    char buf[256];
                    ceps->read([&](const wav_frame_t& w) {
                        if (w.channel >= 1) {
                            auto data = w[0];
                            fullRenderer.height = w.size;
                            ++fullRenderer.width;
                            std::vector<float> block;
                            block.resize(w.size);
                            for (int i = 0; i < w.size; ++i) {
                                auto value = data[i];
                                block[i] = value;
                            }
                            fullRenderer.data_length.push_back(std::move(block));
                        }
                        ++numFrame;
                    });
                    fullRenderer.updateDate();
                    snprintf(buf, sizeof(buf), "频谱已更新（%d帧）", numFrame);
                    global->scriptConsole.push_back(buf);
                    break;
                } else {
                    auto spec = std::dynamic_pointer_cast<spectrumBuilder_t>(it);
                    if (spec != nullptr) {
                        fullRenderer.height = 0;
                        fullRenderer.width = 0;
                        fullRenderer.data_length.clear();
                        fullRenderer.data_phase.clear();
                        fullRenderer.minFreq = 0;
                        fullRenderer.maxFreq = 0;
                        int numFrame = 0;
                        char buf[256];
                        spec->read([&](const spectrum_t& w) {
                            if (w.channel >= 1) {
                                auto data = w[0];
                                int len = w.size / 2;
                                fullRenderer.height = len;
                                ++fullRenderer.width;
                                std::vector<float> block_length;
                                std::vector<float> block_phase;
                                block_length.resize(len);
                                block_phase.resize(len);
                                for (int i = 0; i < len; ++i) {
                                    auto value = sqrt(data[i].r * data[i].r + data[i].i * data[i].i);
                                    block_length[i] = value;
                                    block_phase[i] = atan(data[i].i / data[i].r);
                                }
                                fullRenderer.data_length.push_back(std::move(block_length));
                                fullRenderer.data_phase.push_back(std::move(block_phase));
                            }
                            ++numFrame;
                        });
                        fullRenderer.updateDate();
                        snprintf(buf, sizeof(buf), "频谱已更新（%d帧）", numFrame);
                        global->scriptConsole.push_back(buf);
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
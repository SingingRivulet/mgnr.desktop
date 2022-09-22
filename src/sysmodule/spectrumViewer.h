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
    int specCursor = -1;
    bool fullSpecWindowMoveable = true;
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
                global->checkfocus();
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
                if (specCursor > 0) {
                    static ImVec4 colf = ImVec4(0.0f, 1.0f, 0.4f, 1.0f);
                    const ImU32 col = ImColor(colf);
                    int cursorPos = (specCursor - fullRenderer.viewPort.lookAtBegin.x) / fullRenderer.viewPort.scale;
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
                                specCursor = spos;
                                stepRenderer.get(specCursor);
                            }
                        }
                    } else {
                        fullSpecWindowMoveable = true;
                    }
                }
            }
            ImGui::End();
            if (specCursor > 0) {
                ImGui::SetNextWindowSize(ImVec2(925, 500), ImGuiCond_FirstUseEver);
                snprintf(title, sizeof(title), "频谱显示(时刻)##%d", this->windowId,
                         ImGuiWindowFlags_NoFocusOnAppearing);
                if (ImGui::Begin(title)) {
                    global->checkfocus();
                    //ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
                    ImVec2 p0 = ImGui::GetCursorScreenPos() + ImVec2(20, 20);
                    int w = ImGui::GetWindowWidth() - 40;
                    int h = ImGui::GetWindowHeight() - 60;
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
                    fullRenderer.data.clear();
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
                            fullRenderer.data.push_back(std::move(block));
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
                        fullRenderer.data.clear();
                        int numFrame = 0;
                        char buf[256];
                        spec->read([&](const spectrum_t& w) {
                            if (w.channel >= 1) {
                                auto data = w[0];
                                int len = w.size / 2;
                                fullRenderer.height = len;
                                ++fullRenderer.width;
                                std::vector<float> block;
                                block.resize(len);
                                for (int i = 0; i < len; ++i) {
                                    auto value = sqrt(data[i].r * data[i].r + data[i].i * data[i].i);
                                    block[i] = value;
                                    ;
                                }
                                fullRenderer.data.push_back(std::move(block));
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
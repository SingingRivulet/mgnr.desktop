#include "editWindow.h"

void editWindow::drawSynthUI() {
    if (show_synth_window) {
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("合成器设置", &show_synth_window)) {
            parent->checkfocus();

            if (ImGui::BeginTabBar("合成器设置##tab")) {
                if (ImGui::BeginTabItem("虚拟乐器##tab")) {
                    {
                        ImGui::BeginTable("虚拟乐器##table", 3);
                        int id = 0;
                        char buf[256];
                        for (auto& it : midiSynthesizer.ins.ins) {
                            ImGui::TableNextRow();

                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(it->name.c_str());

                            ImGui::TableNextColumn();
                            snprintf(buf, sizeof(buf), "##up%d", id);
                            if (ImGui::ArrowButton(buf, ImGuiDir_Up)) {
                                midiSynthesizer.ins.swap(id, id - 1);
                            }
                            ImGui::SameLine();
                            snprintf(buf, sizeof(buf), "##down%d", id);
                            if (ImGui::ArrowButton(buf, ImGuiDir_Down)) {
                                midiSynthesizer.ins.swap(id, id + 1);
                            }

                            ImGui::TableNextColumn();
                            snprintf(buf, sizeof(buf), "设置##%d", id);
                            if (ImGui::Button(buf)) {
                                it->show_setting = true;
                            }
                            ++id;
                        }
                        for (auto& it : midiSynthesizer.ins.ins) {
                            if (it->show_setting) {
                                it->settingWindow();
                            }
                        }
                        ImGui::EndTable();
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("效果器##tab")) {
                    {
                        ImGui::BeginTable("效果器##table", 3);
                        int id = 0;
                        char buf[256];
                        for (auto& it : midiSynthesizer.eff.eff) {
                            ImGui::TableNextRow();

                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(it->name.c_str());

                            ImGui::TableNextColumn();
                            snprintf(buf, sizeof(buf), "##up%d", id);
                            if (ImGui::ArrowButton(buf, ImGuiDir_Up)) {
                                midiSynthesizer.eff.swap(id, id - 1);
                            }
                            ImGui::SameLine();
                            snprintf(buf, sizeof(buf), "##down%d", id);
                            if (ImGui::ArrowButton(buf, ImGuiDir_Down)) {
                                midiSynthesizer.eff.swap(id, id + 1);
                            }

                            ImGui::TableNextColumn();
                            snprintf(buf, sizeof(buf), "设置##%d", id);
                            if (ImGui::Button(buf)) {
                                it->show_setting = true;
                            }
                            ++id;
                        }
                        for (auto& it : midiSynthesizer.eff.eff) {
                            if (it->show_setting) {
                                it->settingWindow();
                            }
                        }
                        ImGui::EndTable();
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }
}
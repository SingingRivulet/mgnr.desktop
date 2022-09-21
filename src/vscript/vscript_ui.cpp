#include "vscript_ui.h"
namespace mgnr::vscript {

void node_ui::draw() {
}

void script_ui::draw(bool* showing) {
    focused = false;
    if (ImGui::Begin(title.c_str(), showing)) {
        ImNodes::BeginNodeEditor();

        //添加节点
        onAddNode();

        node_selected.clear();

        for (auto& node : nodes) {
            ImNodes::BeginNode(node.first);

            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted(node.second->name.c_str());
            if (!node.second->errors.empty()) {
                ImGui::SameLine();
                auto errnum = node.second->errors.size();
                if (errnum == 1) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "(!)");
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "(%d)", errnum);
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                    ImGui::BeginTooltip();
                    for (auto& err : node.second->errors) {
                        ImGui::TextUnformatted(err.c_str());
                    }
                    ImGui::EndTooltip();
                }
            }
            ImNodes::EndNodeTitleBar();

            for (auto& it : node.second->input) {
                if (!it->data.empty()) {
                    ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(11, 192, 11, 255));
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(32, 255, 32, 255));
                } else {
                    ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(11, 109, 191, 255));
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(45, 160, 223, 255));
                }
                ImNodes::BeginInputAttribute(it->id);
                ImGui::TextUnformatted(it->name.c_str());
                ImNodes::EndInputAttribute();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }

            node.second->draw();

            if (node.second->indent < 0) {
                //计算最大长度
                for (auto& it : node.second->output) {
                    float len = ImGui::CalcTextSize(it->name.c_str()).x;
                    if (len > node.second->indent) {
                        node.second->indent = len;
                    }
                }
            }
            for (auto& it : node.second->output) {
                if (it->data != nullptr) {
                    ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(11, 192, 11, 255));
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(32, 255, 32, 255));
                } else {
                    ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(11, 109, 191, 255));
                    ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(45, 160, 223, 255));
                }
                ImNodes::BeginOutputAttribute(it->id);
                if (it->indent < 0) {
                    it->indent = ImGui::CalcTextSize(it->name.c_str()).x;
                }
                ImGui::Indent(120.f + node.second->indent - it->indent);
                ImGui::TextUnformatted(it->name.c_str());
                ImNodes::EndOutputAttribute();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }

            ImNodes::EndNode();

            if (ImNodes::IsNodeSelected(node.first)) {
                node_selected.push_back(node.second.get());
            }
        }

        link_selected.clear();
        for (auto& link : links) {
            if (link.second->from->data != nullptr || !link.second->to->data.empty()) {
                ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(11, 192, 11, 255));
                ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(32, 255, 32, 255));
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(24, 233, 24, 255));
            } else {
                ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(11, 109, 191, 255));
                ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(45, 160, 223, 255));
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(39, 150, 210, 255));
            }
            ImNodes::Link(link.first, link.second->from->id, link.second->to->id);
            if (ImNodes::IsLinkSelected(link.first)) {
                link_selected.push_back(link.second.get());
            }
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
        }
        ImNodes::MiniMap();
        ImNodes::EndNodeEditor();

        //查看节点保存的数据
        int id;
        if (ImNodes::IsPinHovered(&id)) {
            auto ito = ports_output.find(id);
            if (ito != ports_output.end()) {
                port_output* p = ito->second;

                auto type = p->type.empty() ? "无类型" : p->type.c_str();
                if (p->data != nullptr) {
                    std::string value = "无法获取的类型";
                    try {
                        auto d = std::dynamic_pointer_cast<mgnr::vscript::value_int>(p->data);
                        if (d != nullptr) {
                            value = std::to_string(d->data);
                        } else {
                            auto d = std::dynamic_pointer_cast<mgnr::vscript::value_string>(p->data);
                            if (d != nullptr) {
                                value = d->data;
                            }
                        }
                    } catch (std::bad_cast&) {
                    }
                    ImGui::SetTooltip(
                        "类型：%s\n值：%s", type, value.c_str());
                } else {
                    ImGui::SetTooltip(
                        "类型：%s", type);
                }
            }

            auto iti = ports_input.find(id);
            if (iti != ports_input.end()) {
                port_input* p = iti->second;

                auto type = p->type.empty() ? "无类型" : p->type.c_str();
                ImGui::SetTooltip(
                    "类型：%s\n缓冲区数据个数：%d", type, p->data.size());
            }
        }
        if (ImNodes::IsLinkDropped(&id)) {
            auto ito = ports_output.find(id);
            if (ito != ports_output.end()) {
                addNodeAt(ito->second);
            }
        }
        {  //创建连接
            int link_begin, link_to;
            if (ImNodes::IsLinkCreated(&link_begin, &link_to)) {
                //printf("link:%d %d\n", link_begin, link_to);
                addLink(link_begin, link_to);
            }
        }

        {  //删除连接
            int link_id;
            if (ImNodes::IsLinkDestroyed(&link_id)) {
                delLink(link_id);
            }
        }
    }
    auto wpos_min = ImGui::GetWindowPos();
    auto wpos_max = ImVec2(
        wpos_min.x + ImGui::GetWindowWidth(),
        wpos_min.y + ImGui::GetWindowHeight());
    if (ImGui::IsItemFocused() ||
        ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
        ImGui::IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows)) {
        focused = true;
    }
    if (ImGui::IsMouseHoveringRect(wpos_min, wpos_max)) {
        focused = true;
    }
    ImGui::End();
}

}  // namespace mgnr::vscript
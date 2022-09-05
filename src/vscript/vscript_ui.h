#pragma once
#include "vscript.h"
namespace mgnr::vscript {

struct node_ui : public node {
    void draw() override;
};

struct script_ui : public script {
    bool focused;
    std::string title;
    std::vector<node*> node_selected{};
    std::vector<link*> link_selected{};
    void draw(bool* showing = nullptr);
    virtual void addNodeAt(port_output* p) = 0;
    virtual void onAddNode() = 0;
};

}  // namespace mgnr::vscript
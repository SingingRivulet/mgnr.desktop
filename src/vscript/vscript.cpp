#include "vscript.h"

void mgnr::vscript::script::addNode(std::unique_ptr<mgnr::vscript::node> n) {
    n->id = ++current_id;
    for (auto& it : n->input) {
        it->id = ++current_id;
        it->parent = n.get();
        ports_input[it->id] = it.get();
    }
    for (auto& it : n->output) {
        it->id = ++current_id;
        it->parent = n.get();
        it->links.clear();
        ports_output[it->id] = it.get();
    }
    n->parent = this;
    nodes[n->id] = std::move(n);
}

void mgnr::vscript::script::removeNode(mgnr::vscript::node* n) {
    for (auto& it : n->input) {
        ports_input.erase(it->id);
        for (auto l : it->links) {
            auto k = std::pair<int, int>(l->from->id, l->from->id);
            l->from->links.erase(l);
            links_map.erase(k);
            links.erase(l->id);
        }
        it->links.clear();
    }
    for (auto& it : n->output) {
        ports_output.erase(it->id);
        for (auto l : it->links) {
            auto k = std::pair<int, int>(l->from->id, l->from->id);
            l->to->links.erase(l);
            links_map.erase(k);
            links.erase(l->id);
        }
        it->links.clear();
    }
    nodes.erase(n->id);
}

mgnr::vscript::link* mgnr::vscript::script::addLink(
    mgnr::vscript::port* from,
    mgnr::vscript::port* to) {
    auto k = std::pair<int, int>(from->id, to->id);
    if (links_map.find(k) == links_map.end()) {
        std::unique_ptr<link> l(new link);
        l->from = from;
        l->to = to;
        l->id = ++current_id;
        auto p = l.get();
        from->links.insert(p);
        to->links.insert(p);
        links_map[k] = p;
        links[l->id] = std::move(l);
        return p;
    }
    return nullptr;
}

void mgnr::vscript::script::delLink(mgnr::vscript::link* l) {
    auto k = std::pair<int, int>(l->from->id, l->from->id);
    l->from->links.erase(l);
    l->to->links.erase(l);
    links_map.erase(k);
    links.erase(l->id);
}

void mgnr::vscript::script::exec() {
    exec_begin();
    while (exec_running()) {
        exec_step();
    }
}
void mgnr::vscript::script::exec_begin() {
    activeNodes.clear();
    for (auto& it : ports_input) {
        it.second->data = nullptr;
    }
    for (auto& it : ports_output) {
        it.second->data = nullptr;
    }
    for (auto& it : nodes) {
        if (it.second->input.empty()) {
            activeNodes.insert((it.second.get()));
        }
    }
}
void mgnr::vscript::script::exec_step() {
    activeNodes_buffer.clear();
    for (auto& it : activeNodes) {
        activeNodes_buffer.push_back(it);
    }
    activeNodes.clear();
    for (auto& it : activeNodes_buffer) {
        exec_node(it);
    }
}
bool mgnr::vscript::script::exec_running() {
    return activeNodes.empty();
}
void mgnr::vscript::script::exec_node(mgnr::vscript::node* n) {
    n->exec();
    for (auto& input : n->input) {
        input->data = nullptr;
    }
    for (auto& output : n->output) {
        linkNodes_buffer.clear();
        for (auto& link : output->links) {
            link->to->data = output->data;  //传输数据
            linkNodes_buffer.insert(link->to->parent);
        }
        for (auto& it : linkNodes_buffer) {
            //检查是否可以执行
            bool canExex = true;
            for (auto& it : it->input) {
                if (it->data == nullptr) {
                    canExex = false;
                    break;
                }
            }
            if (canExex) {
                activeNodes.insert(it);
            }
        }
    }
}
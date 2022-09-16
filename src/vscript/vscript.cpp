#include "vscript.h"

namespace mgnr::vscript {

node::~node() {
}

node* script::addNode(std::unique_ptr<node> n) {
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
    n->staticAttributeId = ++current_id;
    auto res = n.get();
    nodes[n->id] = std::move(n);
    return res;
}

void script::removeNode(node* n) {
    activeNodes.erase(n);
    for (auto& it : n->input) {
        ports_input.erase(it->id);
        for (auto l : it->links) {
            auto k = std::pair<int, int>(l->from->id, l->to->id);
            l->from->links.erase(l);
            links_map.erase(k);
            links.erase(l->id);
        }
        it->links.clear();
    }
    for (auto& it : n->output) {
        ports_output.erase(it->id);
        for (auto l : it->links) {
            auto k = std::pair<int, int>(l->from->id, l->to->id);
            l->to->links.erase(l);
            links_map.erase(k);
            links.erase(l->id);
        }
        it->links.clear();
    }
    nodes.erase(n->id);
}
void script::removeNode(int id) {
    auto it = nodes.find(id);
    if (it != nodes.end()) {
        removeNode(it->second.get());
    }
}

link* script::addLink(
    port_output* from,
    port_input* to) {
    if (from->type != to->type && !to->type.empty()) {
        return nullptr;
    }
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

link* script::addLink(int from, int to) {
    auto itf = ports_output.find(from);
    auto itt = ports_input.find(to);
    if (itf != ports_output.end() &&
        itt != ports_input.end()) {
        return addLink(itf->second, itt->second);
    }
    return nullptr;
}

void script::delLink(link* l) {
    auto k = std::pair<int, int>(l->from->id, l->to->id);
    l->from->links.erase(l);
    l->to->links.erase(l);
    links_map.erase(k);
    links.erase(l->id);
}
void script::delLink(int from, int to) {
    auto k = std::pair<int, int>(from, to);
    auto it = links_map.find(k);
    if (it != links_map.end()) {
        auto l = it->second;
        l->from->links.erase(l);
        l->to->links.erase(l);
        links_map.erase(it);
        links.erase(l->id);
    }
}
void script::delLink(int id) {
    auto it = links.find(id);
    if (it != links.end()) {
        auto l = it->second.get();
        auto k = std::pair<int, int>(l->from->id, l->to->id);
        l->from->links.erase(l);
        l->to->links.erase(l);
        links_map.erase(k);
        links.erase(it);
    }
}

void script::exec(node* n) {
    activeNodes.insert(n);
}
void script::exec_begin() {
    activeNodes.clear();
    printf("mgenner:vscript init\n");
    for (auto& it : ports_input) {
        it.second->data.clear();
    }
    for (auto& it : ports_output) {
        it.second->data = nullptr;
    }
    for (auto& it : nodes) {
        it.second->errors.clear();
    }
}

void script::exec_loop() {
    for (auto it : removeList) {
        if (it->removeable) {
            removeNode(it);
        }
    }
    removeList.clear();
    if (reset) {
        exec_begin();
        reset = false;
    }
}

void port_output::send(std::shared_ptr<value> d) {
    data = d;
    for (auto& link : links) {
        link->to->data.push_back(data);  //传输数据
        parent->affect.insert(link->to->parent);
    }
}

void script::exec_step() {
    activeNodes_buffer.clear();
    for (auto& it : activeNodes) {
        activeNodes_buffer.push_back(it);
    }
    activeNodes.clear();
    for (auto& it : activeNodes_buffer) {
        exec_node(it);
    }
}
bool script::exec_running() {
    return !activeNodes.empty();
}
void script::exec_node(node* n) {
    n->exec();
    for (auto& output : n->output) {
        for (auto& it : n->affect) {
            //检查是否可以执行
            bool canExex = true;
            if (it->needFullInput) {
                for (auto& it : it->input) {
                    if (it->data.empty() && (!it->links.empty())) {
                        canExex = false;
                        break;
                    }
                }
            }
            if (canExex) {
                activeNodes.insert(it);
            }
        }
        n->affect.clear();
    }
}

script::~script() {}
value::~value() {}
}  // namespace mgnr::vscript
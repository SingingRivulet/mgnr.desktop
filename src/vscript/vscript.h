#pragma once
#include <map>
#include <memory>
#include <set>
#include <variant>
#include <vector>
#include "imnodes.h"
#include "imnodes_internal.h"
namespace mgnr::vscript {

struct port;
struct script;
struct node;

struct value {};

struct link {
    friend class script;

   protected:
    port *from, *to;
    int id;
};

struct port {
    friend class script;
    node* parent;
    std::string type;
    std::set<link*> links;
    std::shared_ptr<value> data{};

   protected:
    int id;
};

struct node {
    friend class script;
    script* parent;
    std::vector<std::unique_ptr<port>> input;
    std::vector<std::unique_ptr<port>> output;
    virtual void exec() = 0;
    virtual ~node() = 0;

   protected:
    int id;
};

struct script {
    std::map<int, std::unique_ptr<node>> nodes{};
    std::map<int, std::unique_ptr<link>> links{};
    std::map<std::pair<int, int>, link*> links_map{};

    void addNode(std::unique_ptr<node> n);
    void removeNode(node* n);
    link* addLink(port* from, port* to);
    void delLink(link* l);
    void exec();
    void exec_begin();
    void exec_step();
    bool exec_running();

   protected:
    int current_id = 0;
    std::vector<node*> activeNodes_buffer{};
    std::set<node*> activeNodes{};
    std::set<node*> linkNodes_buffer;
    void exec_node(node* n);
    std::map<int, port*> ports_input{};
    std::map<int, port*> ports_output{};
};

}  // namespace mgnr::vscript

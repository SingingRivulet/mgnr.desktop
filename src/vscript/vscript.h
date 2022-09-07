#pragma once
#include <any>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "imnodes.h"
#include "imnodes_internal.h"
namespace mgnr::vscript {

struct port_input;
struct port_output;
struct script;
struct node;

struct value {
    std::any data;
};

struct link {
    port_output* from;
    port_input* to;

    int id;
};

struct port_input {
    node* parent;
    std::string type;
    std::string name;
    std::set<link*> links;
    std::list<std::shared_ptr<value>> data{};

    float indent = -1;

    int id;
};
struct port_output {
    node* parent;
    std::string type;
    std::string name;
    std::set<link*> links;
    std::shared_ptr<value> data{};

    void send(std::shared_ptr<value> d);

    float indent = -1;

    int id;
};

struct node {
    friend class script;
    script* parent;
    std::string name;
    float indent = -1;
    bool removeable = true;
    bool needFullInput = true;
    std::vector<std::unique_ptr<port_input>> input{};
    std::vector<std::unique_ptr<port_output>> output{};
    std::set<node*> affect{};
    virtual void exec() = 0;
    virtual void draw() = 0;
    virtual ~node();

    int id;
    int staticAttributeId;
};

struct script {
    std::map<int, std::unique_ptr<node>> nodes{};
    std::map<int, std::unique_ptr<link>> links{};
    std::map<std::pair<int, int>, link*> links_map{};

    node* addNode(std::unique_ptr<node> n);
    void removeNode(node* n);
    void removeNode(int id);
    link* addLink(port_output* from, port_input* to);
    link* addLink(int from, int to);
    void delLink(link* l);
    void delLink(int from, int to);
    void delLink(int id);
    void exec(node* n);
    void exec_begin();
    void exec_step();
    bool exec_running();
    void exec_loop();

    bool reset = true;

    std::vector<node*> removeList{};

   protected:
    int current_id = 0;
    std::vector<node*> activeNodes_buffer{};
    std::set<node*> activeNodes{};
    void exec_node(node* n);
    std::map<int, port_input*> ports_input{};
    std::map<int, port_output*> ports_output{};
};

}  // namespace mgnr::vscript

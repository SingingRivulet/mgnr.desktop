#pragma once
#include <memory>
#include <mutex>
#include "dataBlock.h"
//虚拟乐器
namespace mgnr::synthesizer::vinstrument {

struct instrument {
    std::string name;
    bool show_setting = false;
    virtual void render(dataBlock* buffer) = 0;
    virtual bool play_noteOn(note*) = 0;
    virtual bool play_noteOff(note*) = 0;
    virtual void play_stopAll() = 0;
    virtual void settingWindow() = 0;
    virtual instrument* clone() = 0;
    virtual ~instrument();
};

struct manager {
    std::vector<std::shared_ptr<instrument>> ins{};
    inline manager() {
    }
    inline manager(manager& m) {
        ins.clear();
        for (auto& it : m.ins) {
            std::shared_ptr<instrument> p(it->clone());
            ins.push_back(std::move(p));
        }
    }
    void play_noteOn(note*);
    void play_noteOff(note*);
    void play_stopAll();
    void render(dataBlock* buffer);  //依次执行ins，并且混音
    inline void swap(int a, int b) {
        try {
            std::swap(ins.at(a), ins.at(b));
        } catch (...) {
        }
    }
};

}  // namespace mgnr::synthesizer::vinstrument
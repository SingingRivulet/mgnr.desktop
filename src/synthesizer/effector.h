#pragma once
#include <memory>
#include <mutex>
#include "dataBlock.h"
//效果器
namespace mgnr::synthesizer::effector {

struct effector {
    std::string name;
    bool show_setting = false;
    virtual void settingWindow() = 0;
    virtual void process(dataBlock* buffer) = 0;
    virtual effector* clone() = 0;
    virtual ~effector();
};
struct manager {
    std::vector<std::shared_ptr<effector>> eff;
    inline manager() {
    }
    inline manager(manager& m) {
        eff.clear();
        for (auto& it : m.eff) {
            std::shared_ptr<effector> p(it->clone());
            eff.push_back(std::move(p));
        }
    }
    void process(dataBlock* buffer);  //依次处理
    inline void swap(int a, int b) {
        try {
            std::swap(eff.at(a), eff.at(b));
        } catch (...) {
        }
    }
};

}  // namespace mgnr::synthesizer::effector
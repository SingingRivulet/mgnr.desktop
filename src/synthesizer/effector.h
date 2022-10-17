#pragma once
#include <memory>
#include <mutex>
#include "dataBlock.h"
//效果器
namespace mgnr::synthesizer::effector {

struct effector {
    virtual void process(dataBlock* buffer) = 0;
    virtual ~effector();
};
struct manager {
    std::vector<std::shared_ptr<effector>> eff;
    void process(dataBlock* buffer);  //依次处理
};

}  // namespace mgnr::synthesizer::effector
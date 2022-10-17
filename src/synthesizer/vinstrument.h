#pragma once
#include <memory>
#include <mutex>
#include "dataBlock.h"
//虚拟乐器
namespace mgnr::synthesizer::vinstrument {

struct instrument {
    virtual void render(dataBlock* buffer) = 0;
    virtual bool play_noteOn(note*) = 0;
    virtual bool play_noteOff(note*) = 0;
    virtual void play_stopAll() = 0;
    virtual ~instrument();
};

struct manager {
    std::vector<std::shared_ptr<instrument>> ins{};
    void play_noteOn(note*);
    void play_noteOff(note*);
    void play_stopAll();
    void render(dataBlock* buffer);  //依次执行ins，并且混音
};

}  // namespace mgnr::synthesizer::vinstrument
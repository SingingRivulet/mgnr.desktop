#pragma once
#include <sox.h>
#include <memory>
#include <mutex>
#include "effector.h"
#include "vinstrument.h"
//合成器
namespace mgnr::synthesizer {

struct synthesizer {
    synthesizer();
    synthesizer(synthesizer&);
    ~synthesizer();
    vinstrument::manager ins;
    effector::manager eff;
    bool running = false;
    inline void play_noteOn(note* n) {
        ins.play_noteOn(n);
    }
    inline void play_noteOff(note* n) {
        ins.play_noteOff(n);
    }
    inline void play_start() {
        running = true;
        ins.play_stopAll();
    }
    inline void play_stop() {
        running = false;
        ins.play_stopAll();
    }
};

}  // namespace mgnr::synthesizer
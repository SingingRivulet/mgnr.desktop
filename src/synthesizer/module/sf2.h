#pragma once
#include <fluidsynth.h>
#include "synthesizer/vinstrument.h"
namespace mgnr::synthesizer::vinstrument {

struct sf2 : instrument {
    fluid_settings_t* settings = nullptr;
    fluid_synth_t* synth = nullptr;

    int playNum[16];
    int playIns[16];
    int ins2Channel[128];

    std::unordered_map<std::string, int> instrument2Id;
    long playTimes = 0;

    void render(dataBlock* buffer) override;
    bool play_noteOn(note*) override;
    bool play_noteOff(note*) override;
    void play_stopAll() override;
    int useInstrument(const stringPool::stringPtr& n);
    int releaseInstrument(const stringPool::stringPtr& n);
    int getInstrumentId(const stringPool::stringPtr& name);
    sf2(const char* path);
    ~sf2() override;
};

}  // namespace mgnr::synthesizer::vinstrument
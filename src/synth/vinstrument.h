#pragma once
#include <memory>
#include <mutex>
#include "editTable.h"
namespace mgnr::offlineSynth::vinstrument {
class instrument {
   public:
    instrument* next;
    virtual bool play_noteOn(note*) = 0;
    virtual bool play_noteOff(note*) = 0;
};
class manager {
    std::vector<std::shared_ptr<instrument>> ins;

   public:
    void addIns(const std::shared_ptr<instrument>& ptr);
    void clear();
    void play_noteOn(note*);
    void play_noteOff(note*);
    void render(int* buffer);
};
}  // namespace mgnr::offlineSynth::vinstrument
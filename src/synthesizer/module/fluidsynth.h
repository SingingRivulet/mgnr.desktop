#include "synthesizer/vinstrument.h"
namespace mgnr::synthesizer::vinstrument {

struct fluidsynth : instrument {
    void render(dataBlock* buffer) override;
    bool play_noteOn(note*) override;
    bool play_noteOff(note*) override;
    void play_stopAll() override;
    ~fluidsynth() override;
};

}  // namespace mgnr::synthesizer::vinstrument
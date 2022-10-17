#include "vinstrument.h"
namespace mgnr::synthesizer::vinstrument {

instrument::~instrument() {}

void manager::play_noteOn(note* n) {
    for (auto& it : ins) {
        if (it->play_noteOn(n)) {
            break;
        }
    }
}

void manager::play_noteOff(note* n) {
    for (auto& it : ins) {
        if (it->play_noteOff(n)) {
            break;
        }
    }
}

void manager::play_stopAll() {
    for (auto& it : ins) {
        it->play_stopAll();
    }
}

void manager::render(dataBlock* buffer) {
    for (auto& it : ins) {
        it->render(buffer);
    }
}

}  // namespace mgnr::synthesizer::vinstrument
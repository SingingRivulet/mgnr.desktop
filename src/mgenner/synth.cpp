#include "synth.h"
namespace mgnr {

synth::~synth() {
    clearTracks();
}

void synth::onUseInfo(const stringPool::stringPtr& info) {
    if (info.empty())
        return;
    if (info.at(0) == '@')
        return;
    onLoadName(info);
    loadInstrument(info);
}

}  // namespace mgnr

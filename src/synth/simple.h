#pragma once
#include <memory>
#include <mutex>
#include "WavFile.h"
#include "offline.h"
//简易合成器（不使用任何插件）
namespace mgnr::offlineSynth::simple {

inline void render(midiMap* midi,
                   const char* sf,
                   int sampleRate,
                   const char* outpath) {
    mgnr::offline renderer(sf, sampleRate);
    for (auto it : midi->notes) {
        renderer.addNote(it->begin, it->tone, it->duration, it->volume, it->info);
    }
    renderer.timeMap = midi->timeMap;
    renderer.updateTimeMax();
    float buf[64];
    auto fp = fopen(outpath, "w");
    if (fp) {
        WavOutFile out(fp, sampleRate, 16, 1);
        while (renderer.renderStep(buf)) {
            out.write(buf, 64);
        }
        fclose(fp);
    }
}

}  // namespace mgnr::offlineSynth::simple
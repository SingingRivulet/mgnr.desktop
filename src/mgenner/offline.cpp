//
// Created by admin on 2022/2/24.
//
#define TSF_IMPLEMENTATION
#include "offline.h"

namespace mgnr {

void offline::rebuildNoteLen() {}

void offline::drawNote_begin() {}

void offline::drawNote(int fx, int fy, int tx, int ty, int volume, const stringPool::stringPtr& info, bool selected, bool onlydisplay) {}

void offline::drawNote_end() {}

void offline::drawDescriptions(float p, const stringPool::stringPtr& title, const std::string& content) {}

void offline::drawDescriptionsPadd() {}

void offline::drawCaption(float p, const std::string& str) {}

void offline::drawTableRaw(int from, int to, int left, int right, int t) {}

void offline::drawTimeCol(float p) {}

void offline::drawSectionCol(float p, int n) {}

void offline::drawTempo(float p, double t) {}

void offline::drawTempoPadd() {}

void offline::drawScroll() {}

void offline::scrollBuilder_onGetNoteArea() {}

void offline::scrollBuilder_onGetAllNotePos(note*) {}

void offline::scrollBuilder_onSwap() {}

void offline::onLoadName(const stringPool::stringPtr& name) {}

void offline::onSelectedChange(int len) {}

void offline::editStatusUpdate() {}

long offline::getTime() {
    return (long)nowTime;
}

offline::offline() {
}

offline::~offline() {
}

bool offline::renderStep(float* buffer) {
    nowTime = nowTime_point * 1000 / 44100;
    //::__android_log_print(ANDROID_LOG_INFO,
    //                      "offline_render",
    //                      "nowTime:%f nowTime_point:%d", nowTime, nowTime_point);
    if (!playingStatus) {
        playStart();
    }
    playStep();
    synthesizer::dataBlock block;
    bzero(&block, sizeof(block));
    midiSynthesizer.ins.render(&block);
    midiSynthesizer.eff.process(&block);
    for (int i = 0; i < 512; ++i) {
        buffer[i * 2 + 0] = block.buffer_channel[0][i];
        buffer[i * 2 + 1] = block.buffer_channel[1][i];
    }
    nowTime_point += 512;
    return noteTimeMax > lookAtX;
}
}  // namespace mgnr

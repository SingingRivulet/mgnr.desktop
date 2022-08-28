//
// Created by admin on 2022/2/24.
//

#ifndef MIDILIB_OFFLINE_H
#define MIDILIB_OFFLINE_H

#include "synth.h"
#include "tsf.h"

namespace mgnr {

    class offline : public synth {
    private:
        tsf *soundfont;
        double nowTime = 0;
        double sampleRate = 44100;
        int nowTime_point = 0;
    public:
        offline(const char *sf, int sampleRate = 44100);

        ~offline();

        void rebuildNoteLen() override;

        void drawNote_begin() override;

        void drawNote(int fx, int fy, int tx, int ty, int volume, const stringPool::stringPtr &info,
                      bool selected, bool onlydisplay = false) override;

        void drawNote_end() override;

        void drawTableRaw(int from, int to, int left, int right, int t) override;

        void drawTimeCol(float p) override;

        void drawSectionCol(float p, int n) override;

        void drawTempo(float p, double t) override;

        void drawTempoPadd() override;

        void drawScroll() override;

        void drawDescriptions(float p, const stringPool::stringPtr &title,
                              const std::string &content) override;

        void drawDescriptionsPadd() override;

        void drawCaption(float p, const std::string &str) override;

        void scrollBuilder_onGetNoteArea() override;

        void scrollBuilder_onGetAllNotePos(note *) override;

        void scrollBuilder_onSwap() override;

        void onNoteOn(note *n, int c) override;

        void onNoteOff(note *n, int c) override;

        void onSetChannelIns(int c, int ins) override;

        void callSynthNoteOn(const char *info, int channel, int tone, int vol) override;

        void callSynthNoteOff(const char *info, int channel, int tone) override;

        void onLoadName(const stringPool::stringPtr &name) override;

        void onSelectedChange(int len) override;

        int getTime() override;

        bool renderStep(float *buffer);
    };
}

#endif //MIDILIB_OFFLINE_H

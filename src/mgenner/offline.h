//
// Created by admin on 2022/2/24.
//

#ifndef MIDILIB_OFFLINE_H
#define MIDILIB_OFFLINE_H

#include "synth.h"

namespace mgnr {

class offline : public synth {
   private:
    double nowTime = 0;
    long long nowTime_point = 0;

   public:

    offline();

    ~offline();

    void rebuildNoteLen() override;

    void drawNote_begin() override;

    void drawNote(int fx, int fy, int tx, int ty, int volume, const stringPool::stringPtr& info, bool selected, bool onlydisplay = false) override;

    void drawNote_end() override;

    void drawTableRaw(int from, int to, int left, int right, int t) override;

    void drawTimeCol(float p) override;

    void drawSectionCol(float p, int n) override;

    void drawTempo(float p, double t) override;

    void drawTempoPadd() override;

    void drawScroll() override;

    void drawDescriptions(float p, const stringPool::stringPtr& title, const std::string& content) override;

    void drawDescriptionsPadd() override;

    void drawCaption(float p, const std::string& str) override;

    void scrollBuilder_onGetNoteArea() override;

    void scrollBuilder_onGetAllNotePos(note*) override;

    void scrollBuilder_onSwap() override;

    void onLoadName(const stringPool::stringPtr& name) override;

    void onSelectedChange(int len) override;

    long getTime() override;

    bool renderStep(float* buffer);

    void editStatusUpdate() override;
};
}  // namespace mgnr

#endif  //MIDILIB_OFFLINE_H

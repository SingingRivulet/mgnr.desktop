#pragma once
#include "renderContext.h"
class editWindow : public mgnr::synth {
   public:
    int windowId;

    mgnr::stringPool::stringPtr lastDefaultInfo;
    int lastSection = -1;

    SDL_Texture* scroll_texture_buffer = nullptr;
    SDL_Texture* scroll_texture = nullptr;

    renderContext* parent;

    char defaultInfoBuffer[128];

    bool needUpdateWindowTitle = true;
    void updateWindowTitle_process();

    void onSetDefaultInfo(const mgnr::stringPool::stringPtr& info);

    void onUseInfo(const mgnr::stringPool::stringPtr& info) override;

    void onSetSection(int sec);

    double noteWidth = 1.0;
    void rebuildNoteLen() override;

    void onLoadName(const mgnr::stringPool::stringPtr& name) override;
    void setInfo(const std::string& str);

    void editStatusUpdate() override;

    void playStep();

    std::string midiFilePath = "";
    inline static const char* getExt(const char* name, const char c) {
        auto out = strrchr(name, c);
        if (out == nullptr) {
            return nullptr;
        } else {
            ++out;
            return out;
        }
    }
    inline static bool checkExt(const char* name, const char* ext) {
        auto e = getExt(name, '.');
        return (e && strcmp(e, ext) == 0);
    }
    inline static const char* getFileName(const char* name) {
        auto out = getExt(name, '/');
        if (out == nullptr) {
            return name;
        } else {
            return out;
        }
    }
    inline void setMidiFilePath(const std::string& name) {
        midiFilePath = name;
        fileName = getFileName(name.c_str());
        needUpdateWindowTitle = true;
    }
    std::string fileName = "";

    bool show_edit_window = false;
    bool show_trackSelect_window = false;

    int noteWidth_items_id = 3;

    //////////////////////////////////////////////////////////////////
    //绘制
    void drawNote_begin() override;
    void drawNote(int fx, int fy, int tx, int ty, int volume, const mgnr::stringPool::stringPtr& info, bool selected, bool onlydisplay) override;
    void drawNote_end() override;
    void drawTableRaw(int from, int to, int left, int right, int t) override;
    void drawTimeCol(float p) override;
    void drawSectionCol(float p, int n) override;
    void drawTempo(float p, double t) override;
    void drawDescriptions(float p, const mgnr::stringPool::stringPtr& title, const std::string& content) override;
    void drawDescriptionsPadd() override;
    void drawTempoPadd() override;
    void drawScroll() override;
    void drawCaption(float p, const std::string& s) override;
    void draw();
    void hideMode();
    //////////////////////////////////////////////////////////////////
    //滚动条
    int hlen;
    int nmax;
    int nmin;
    void scrollBuilder_onGetNoteArea() override;
    void scrollBuilder_onGetAllNotePos(mgnr::note* n) override;
    void scrollBuilder_onSwap() override;
    void buildScroll();

    void onSetChannelIns(int c, int ins) override;
    void callSynthNoteOn(const char* info, int channel, int tone, int vol) override;
    void callSynthNoteOff(const char* info, int channel, int tone) override;
    void onSelectedChange(int len) override;
    void onScriptCmd(const char* cmd) override;

    editWindow(renderContext* p);
    ~editWindow();
};
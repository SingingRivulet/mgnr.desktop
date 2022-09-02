#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <dlfcn.h>
#include <fluidsynth.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include <filesystem>
#include <iostream>
#include "imfilebrowser.h"
#include "lua.hpp"
#include "luacall.h"
#include "sampler.h"
#include "synth.h"

class mgenner : public mgnr::synth {
   public:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    std::unordered_map<std::string, std::array<unsigned char, 3>> colors;
    std::unordered_map<std::string,
                       std::map<std::tuple<int, int, int>,
                                std::tuple<SDL_Texture*, int, int>>>
        words;

    SDL_Texture* scroll_texture_buffer = nullptr;
    SDL_Texture* scroll_texture = nullptr;

    std::tuple<SDL_Texture*, int, int> toneMap[128];

    bool running = true;

    bool selectByBox = false;
    bool selectingByBox = false;
    int selectBoxX;
    int selectBoxY;
    int selectBoxXend;
    int selectBoxYend;

    mgenner();
    ~mgenner();

    mgnr::stringPool::stringPtr lastDefaultInfo;
    int lastSection = -1;

    void toneMapInit();

    int mouse_x;
    int mouse_y;
    bool focusCanvas;
    std::vector<SDL_Event> events;
    bool button_ctrl = false;
    bool button_shift = false;
    void loop();
    void processEvents();

    void onSetDefaultInfo(const mgnr::stringPool::stringPtr& info);

    void onUseInfo(const mgnr::stringPool::stringPtr& info) override;

    void onSetSection(int sec);

    double noteWidth = 1.0;
    void rebuildNoteLen() override;

    void onLoadName(const mgnr::stringPool::stringPtr& name) override;
    void setInfo(const std::string& str);

    void editStatusUpdate() override;

    void updateWindowTitle();

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
    SDL_Texture* getText(const std::string& str, const SDL_Color& textColor, SDL_Rect& rect);
    //////////////////////////////////////////////////////////////////
    //滚动条
    int hlen;
    int nmax;
    int nmin;
    void scrollBuilder_onGetNoteArea() override;
    void scrollBuilder_onGetAllNotePos(mgnr::note* n) override;
    void scrollBuilder_onSwap() override;
    void buildScroll();
    //////////////////////////////////////////////////////////////////
    ///ui相关
    bool show_edit_window = false;
    bool show_trackSelect_window = false;

    bool show_tempoSet_bar = false;
    int show_tempoSet_bar_pos_x;
    int show_tempoSet_bar_pos_y;

    bool show_tempoAdd_bar = false;
    int show_tempoAdd_bar_val;

    bool show_midiDescription_bar = false;
    bool show_descriptionAdd_bar = false;
    bool show_descriptionEdit_bar = false;
    bool focus_note = false;
    int show_midiDescription_bar_pos_x;
    int show_midiDescription_bar_pos_y;
    int midiDescriptionBuffer_tick;
    int noteWidth_items_id = 3;
    char midiDescriptionBuffer_content[4096];
    std::string midiDescriptionBuffer_title;

    bool show_trackMap_window = false;
    std::list<std::tuple<std::string, int, int>> trackMapBuffer;
    bool trackMapBuffer_closeWithSave = false;
    int trackMapBuffer_setInstrument_track;
    int trackMapBuffer_setInstrument_ins;
    void trackMapBuffer_init(std::map<std::string, int>& trackNameMapper,
                             std::map<int, int>& trackInsMapper);
    void trackMapBuffer_save();

    ImGui::FileBrowser fileDialog_loadMidi;
    ImGui::FileBrowser fileDialog_saveMidi;

    char defaultInfoBuffer[128];
    void ui_init();
    void ui_loop();
    void ui_shutdown();

    std::string midiFilePath = "";

    void loadMidiDialog();
    void saveMidiDialog();
    void loadMidiFile(const std::string& path);
    void saveMidiFile(const std::string& path);
    void saveMidiFile();

    inline void checkfocus() {
        if (ImGui::IsItemFocused() || ImGui::IsWindowFocused() || ImGui::IsWindowHovered()) {
            focusCanvas = false;
        }
    }
    /////////////////////////////////////////////////////////////////////////////
    //合成器
    fluid_settings_t* settings = nullptr;
    fluid_synth_t* synth = nullptr;
    fluid_audio_driver_t* adriver = nullptr;
    int sfont_id;
    void synth_init();
    void synth_shutdown();
    void playStep();
    void onSetChannelIns(int c, int ins) override;
    void callSynthNoteOn(const char* info, int channel, int tone, int vol) override;
    void callSynthNoteOff(const char* info, int channel, int tone) override;
    void onSelectedChange(int len) override;
    void onScriptCmd(const char* cmd) override;
    /////////////////////////////////////////////////////////////////////////////
    //插件
    std::string path_font = "./res/font/font.ttf";
    std::string path_sf2 = "./res/soundfont/sndfnt.sf2";
    struct pluginConfig {
        std::string name;
        int init = -1;
        int shutdown = -1;
        int drawUI = -1;
        int loop = -1;
    };
    std::vector<pluginConfig*> plugins;
    std::set<pluginConfig*> plugins_showing;
    lua_State* lua_mainthread;
    void loadConfig();
    void plugin_menu();
    void plugin_show();
    void plugin_loop();
    void shutdownPlugins();
};

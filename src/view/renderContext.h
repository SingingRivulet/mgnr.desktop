#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <dlfcn.h>
#include <fluidsynth.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include <imnodes.h>
#include <imnodes_internal.h>
#include <filesystem>
#include <iostream>
#include "imfilebrowser.h"
#include "lua.hpp"
#include "luacall.h"
#include "sampler.h"
#include "synth.h"
#include "vscript/vscript_ui.h"
class editWindow;
struct renderContext {
    renderContext();
    ~renderContext();

    std::map<int, std::unique_ptr<editWindow>> editWindows;
    int windows_current_id = 0;
    int createWindow();
    void closeWindow(int id);
    void showWindow(editWindow*);
    void showWindow(int id);

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    std::unordered_map<std::string, std::array<unsigned char, 3>> colors;
    std::unordered_map<std::string,
                       std::map<std::tuple<int, int, int>,
                                std::tuple<SDL_Texture*, int, int>>>
        words;

    std::tuple<SDL_Texture*, int, int> toneMap[128];
    editWindow* drawing = nullptr;
    bool running = true;
    int windowHeight;  //窗口高度
    int windowWidth;   //窗口宽度

    int mouse_x;
    int mouse_y;
    bool focusCanvas;
    std::vector<SDL_Event> events;
    bool button_ctrl = false;
    bool button_shift = false;
    void loop();
    void processEvents();
    void draw();

    void toneMapInit();

    SDL_Texture* getText(const std::string& str, const SDL_Color& textColor, SDL_Rect& rect);
    //////////////////////////////////////////////////////////////////
    //框选
    bool selectByBox = false;
    bool selectingByBox = false;
    int selectBoxX;
    int selectBoxY;
    int selectBoxXend;
    int selectBoxYend;
    //////////////////////////////////////////////////////////////////
    ///ui相关

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
    ImGui::FileBrowser fileDialog_importMidi;

    void loadMidiDialog();
    void saveMidiDialog();
    void loadMidiFile(const std::string& path);
    void saveMidiFile(const std::string& path);
    void saveMidiFile();

    void ui_init();
    void ui_loop();
    void ui_shutdown();
    /////////////////////////////////////////////////////////////////////////////
    //插件
    bool module_showNodeEditor = false;
    std::string path_font = "./res/font/font.ttf";
    std::string path_sf2 = "./res/soundfont/sndfnt.sf2";

    struct vclass_t;
    struct moduleConfig {
        std::string name;
        int init = -1;
        int shutdown = -1;
        int drawUI = -1;
        int loop = -1;
        std::unique_ptr<vclass_t> vclass;
    };
    std::vector<moduleConfig*> modules;
    std::vector<moduleConfig*> modules_loop;
    std::vector<moduleConfig*> modules_haveUI;
    std::set<moduleConfig*> modules_showing;

    lua_State* lua_mainthread;
    void loadConfig();
    void module_menu();
    void module_nodeEditor();
    void module_show();
    void module_loop();
    void shutdownModules();

    inline void checkfocus() {
        if (ImGui::IsItemFocused() ||
            ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
            ImGui::IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows)) {
            focusCanvas = false;
        }
    }
    //节点编辑器
    struct vclass_t {
        bool needFullInput = true;
        std::string name;
        std::vector<std::tuple<std::string, std::string>> inputs, outputs;
        int exec = -1;
        int draw = -1;
    };
    struct vscript_t : mgnr::vscript::script_ui {
        renderContext* global;
        bool addNodeMode = false;
        std::vector<
            std::tuple<
                std::string,
                std::function<mgnr::vscript::node*(vscript_t*)>>>
            scriptClass_other;
        std::map<
            std::string,
            std::vector<
                std::tuple<
                    std::string,
                    std::function<mgnr::vscript::node*(vscript_t*)>>>>
            scriptClass;
        vscript_t();
        void onAddNode() override;
        mgnr::vscript::port_output* addNodeAtPort = nullptr;
        ImVec2 addNodeAtPort_window_pos;
        void addNodeAt(mgnr::vscript::port_output* p) override;
    } vscript;
    std::vector<std::string> scriptConsole{};
    void vscript_init();

    /////////////////////////////////////////////////////////////////////////////
    //合成器
    fluid_settings_t* settings = nullptr;
    fluid_synth_t* synth = nullptr;
    fluid_audio_driver_t* adriver = nullptr;
    int sfont_id;
    void synth_init();
    void synth_shutdown();
    void playStep();
};
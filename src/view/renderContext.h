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
#include <atomic>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <thread>
#include "imfilebrowser.h"
#include "lua.hpp"
#include "luacall.h"
#include "sampler.h"
#include "synth.h"
#include "vscript/vscript_ui.h"
class editWindow;
struct renderContext : public mgnr::clipboard_t {
    renderContext();
    ~renderContext();

    std::map<int, std::unique_ptr<editWindow>> editWindows;

    std::set<editWindow*> synthList;
    std::mutex synthList_locker;
    void synthThread_func();
    std::thread synthThread;

    int windows_current_id = 0;
    std::tuple<int, editWindow*> createWindow();
    void closeWindow(int id);
    void showWindow(editWindow*);
    void showWindow(int id);

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    std::unordered_map<std::string, std::tuple<float, float>> colors;  //采用hsl格式
    std::unordered_map<std::string,
                       std::map<std::tuple<int, int, int>,
                                std::tuple<SDL_Texture*, int, int>>>
        words;

    std::tuple<SDL_Texture*, int, int> toneMap[128];
    editWindow* drawing = nullptr;
    std::atomic<bool> running = true;
    int windowHeight;  //窗口高度
    int windowWidth;   //窗口宽度

    int mouse_x;
    int mouse_y;
    bool focusCanvas;
    bool hoverCanvas;
    std::vector<SDL_Event> events_mouse;
    std::vector<SDL_Event> events_keyboard;
    bool button_ctrl = false;
    bool button_shift = false;
    void loop();
    void processEvents_mouse();
    void processEvents_keyboard();
    void draw();

    void toneMapInit();

    SDL_Texture* getText(const std::string& str, const SDL_Color& textColor, SDL_Rect& rect);

    bool shutdowning = false;
    bool closeWithShutdown = false;
    bool showingCloseWindow = false;
    void shutdown_process();
    void showCloseWindow();
    //////////////////////////////////////////////////////////////////
    //框选
    bool selectByBox = false;
    bool selectingByBox = false;
    int selectBoxX;
    int selectBoxY;
    int moveNoteX;
    int moveNoteY;
    int selectBoxXend;
    int selectBoxYend;
    bool addNoteMode = false;
    bool moveNoteMode = false;
    bool resizeNoteMode = false;
    bool resizeNotehover = false;
    bool resizeNoteReady = false;
    bool selectNoteFail = false;
    //////////////////////////////////////////////////////////////////
    ///ui相关

    int menuHeight = 0;

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
    std::vector<std::tuple<std::string, int, int>> trackMapBuffer;
    std::vector<int> trackMapBuffer_index;
    void sortTrackMapBufferByName();
    void sortTrackMapBufferByTrack();
    void sortTrackMapBufferByInstrument();
    void sortTrackMapBuffer();
    int sortTrackMapBuffer_sortId = 0;
    bool sortTrackMapBuffer_sortInv = false;
    bool trackMapBuffer_closeWithSave = false;
    int trackMapBuffer_setInstrument_track;
    int trackMapBuffer_setInstrument_ins;
    void trackMapBuffer_init(std::map<std::string, int>& trackNameMapper,
                             std::map<int, int>& trackInsMapper);
    void trackMapBuffer_save();

    ImGui::FileBrowser fileDialog_loadMidi;
    ImGui::FileBrowser fileDialog_openMidi;
    ImGui::FileBrowser fileDialog_saveMidi;
    ImGui::FileBrowser fileDialog_exportWav;

    void loadMidiDialog();
    void saveMidiDialog();
    void openMidiDialog();
    void exportWavDialog();
    void loadMidiFile(const std::string& path);
    void openMidiFile(const std::string& path);
    void saveMidiFile(const std::string& path);
    void saveMidiFile();

    void ui_init();
    void ui_loop();
    void ui_shutdown();
    /////////////////////////////////////////////////////////////////////////////
    //插件
    bool module_showNodeEditor = false;
    std::string path_font = "./res/font/font.ttf";
    std::string path_imgui_ini = "./imgui.ini";
    std::string path_imgui_log = "./imgui_log.txt";

    std::string path_sf2 = "./res/soundfont/sndfnt.sf2";

    struct vclass_lua;
    struct moduleConfig {
        std::string name;
        int init = -1;
        int shutdown = -1;
        int drawUI = -1;
        int loop = -1;
        std::unique_ptr<vclass_lua> vclass;
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

    //节点编辑器
    struct vclass_lua {
        bool needFullInput = true;
        std::string name;
        std::vector<std::tuple<std::string, std::string>> inputs, outputs;
        int exec = -1;
        int draw = -1;
    };
    struct vscript_t;
    struct vclass_t {
        std::string title;
        std::string menu;
        std::set<std::string> input_types;
        std::function<mgnr::vscript::node*(vscript_t*)> callback;
    };
    struct vscript_t : mgnr::vscript::script_ui {
        renderContext* global;
        bool addNodeMode = false;
        bool menuPopup = false;
        std::vector<std::shared_ptr<vclass_t>> scriptClass_other;
        std::map<std::string, std::vector<std::shared_ptr<vclass_t>>> scriptClass;
        std::map<std::string, std::vector<std::shared_ptr<vclass_t>>> scriptClass_type;
        std::vector<std::shared_ptr<vclass_t>> scriptClass_view;
        void buildViewClass(const std::string& type);
        vscript_t();
        void onAddNode() override;
        mgnr::vscript::port_output* addNodeAtPort = nullptr;
        ImVec2 addNodeAtPort_window_pos;
        void addNodeAt(mgnr::vscript::port_output* p) override;
    } vscript;
    std::vector<std::string> scriptConsole{};
    void addVClass(std::shared_ptr<vclass_t> p);
    void vscript_init();

    /////////////////////////////////////////////////////////////////////////////
    int sfont_id;
    void playStep();
};
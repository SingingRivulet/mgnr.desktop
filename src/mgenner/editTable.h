#ifndef MGNR_EDIT_TABLE
#define MGNR_EDIT_TABLE
#include <stdio.h>
#include <list>
#include <memory>
#include <sstream>
#include <unordered_set>
#include "midiMap.h"
namespace mgnr {
extern const char* instrumentName[128];
struct noteInfo {
    //音符数据（如果是删除的话）
    float position;
    float tone;
    float duration;
    int volume;
    int id;
    std::string info;
    noteInfo(note* p)
        : info(p->info.value()) {
        position = p->begin;
        tone = p->tone;
        duration = p->duration;
        volume = p->volume;
        id = p->id;
    }
};
struct displayBuffer_t {
    float begin;
    float tone;
    float dur;
    int volume;
    stringPool::stringPtr info;
};
struct clipboard_t {
    std::vector<displayBuffer_t> noteTemplate;
};
struct history : public std::enable_shared_from_this<history> {
    enum {
        H_NOTE_ADD,
        H_NOTE_DEL,
        H_TEMPO_ADD,
        H_TEMPO_DEL
    } method;
    std::list<int> noteIds;  //音符，如果是添加的话，将会存在
    int begin;               //起始时间
    double tempo;            //添加删除速度时使用
    std::list<std::unique_ptr<noteInfo>> notes;
};
class editTable : public midiMap {
   public:
    editTable();
    ~editTable();

    void render();
    void drawDisplay();
    virtual void drawNote_begin() = 0;
    virtual void drawNote(int fx, int fy, int tx, int ty, int volume, const stringPool::stringPtr& info, bool selected, bool onlydisplay = false) = 0;
    virtual void drawNote_end() = 0;

    virtual void editStatusUpdate() = 0;

    std::map<std::string, int> trackNameMapper;
    std::map<int, int> trackInsMapper;
    void resetTrackMapper();
    bool checkTrackMapper(std::map<std::string, int>& trackNameMapper,
                          std::map<int, int>& trackInsMapper);

    HBB::vec screenToAbs(int x, int y);  //屏幕坐标转midi绝对坐标

    note* clickToAdd(int x, int y);
    void addDisplaied();
    int clickToSelect(int x, int y);
    void clickToSetTempo(int x, int y, double tp);
    void clickToRemoveTempo(int x, int y);
    void clickToAddDescription(int x, int y, const std::function<void(int tick)>& callback);
    void clickToSetDescription(int x, int y, const std::function<void(int tick, const stringPool::stringPtr& title, const std::string& content)>& callback);
    void clickToLookAt(int x, int y);
    int selectAll();
    void clearSelected();
    void removeSelected();
    void selectedToRelative(std::string& out);
    void clearNotes();
    void renameSelected(const stringPool::stringPtr& n);
    void resizeSelected(int delta);
    void automatic(float& x, float& y);
    void clickToDisplay(int x, int y);
    void clickToDisplay_close();

    int selectByArea(int x, int y, int len);
    int selectByArea_unique(int x, int y, int len);
    int selectByArea(int selectBoxX, int selectBoxXend, int selectBoxY, int selectBoxYend);

    void drawNoteAbs(note*);  //画音符绝对坐标
    void drawNoteAbs(float begin, float tone, float duration, float volume, const stringPool::stringPtr& info, bool selected, bool onlydisplay = false);
    void findNote();  //根据参数找到搜索矩形，利用HBB找到音符
    void drawTableRaws();
    bool drawToneRaw(int t);
    void drawTableColumns();
    void drawSectionLine();
    void drawTempoLine();
    void drawDescriptionsLine();
    void drawCaptions();

    virtual void drawTableRaw(int from, int to, int left, int right, int t) = 0;
    virtual void drawTimeCol(float p) = 0;
    virtual void drawSectionCol(float p, int n) = 0;
    virtual void drawTempo(float p, double t) = 0;
    virtual void drawTempoPadd() = 0;
    virtual void drawDescriptions(float p, const stringPool::stringPtr& title, const std::string& content) = 0;
    virtual void drawDescriptionsPadd() = 0;
    virtual void drawScroll() = 0;
    virtual void drawCaption(float p, const std::string& str) = 0;
    virtual void onScriptCmd(const char*);

    void toString(std::string& str);
    void loadString(const std::string& str);
    void loadMidi(const std::string& str);
    std::string loadMidi_preprocess(const std::string& str, const std::string& script, int tone);
    std::string exportString();
    void exportMidi(const std::string& str);
    void exportMidiWithTrackMapper(const std::string& filename);

    double lookAtX;  //瞄准位置（左边缘中心点）
    float lookAtY;
    int rawLeft;
    int rawRightMax = -1;
    int rawRight;

    int pitchNum = 128;

    float noteHeight;  //音符图片高度
    float noteLength;  //音符长度比例

    int windowHeight;  //窗口高度
    int windowWidth;   //窗口宽度

    bool automaticX;   //自动吸附模式（起始）
    bool automaticY;   //自动吸附模式（音高）
    float maticBlock;  //起始时间吸附到这个的整数倍

    int baseTone;
    bool isMajor;
    int getBaseTone();  //获取调性

    float defaultDuration;                 //持续时间
    int defaultVolume;                  //音量
    stringPool::stringPtr defaultInfo;  //信息

    std::set<note*>::iterator scrollBuilder_it;
    void scrollBuilder_process();
    bool scrollBuilder_processing = false;
    virtual void scrollBuilder_onGetNoteArea() = 0;
    virtual void scrollBuilder_onGetAllNotePos(note*) = 0;
    virtual void scrollBuilder_onSwap() = 0;

    std::map<int, std::string> captions;

    void addChord(float position,
                  const std::string& root,
                  const std::string& name,
                  const char* format,
                  float length,
                  int root_base = 0,
                  int v = 70,
                  const std::string& info = "default",
                  bool useTPQ = true);
    void addChord(float position, const std::string& name, float length, int root_base = 0, int v = 70, const std::string& info = "default", const std::string& format = "", bool useTPQ = true);

    void addNoteWithId(float position, float tone, float dur, int v, int ins);

    void parseChordNotes(std::vector<int>& notes, const std::string& name) const;
    inline int getNoteId(const std::string& name) const {
        auto it = note_number_map.find(name);
        if (it == note_number_map.end()) {
            return 0;
        } else {
            return it->second;
        }
    }
    inline int getChordMax(const std::string& name) const {
        std::vector<int> notes;
        parseChordNotes(notes, name);
        int len = notes.size();
        if (len <= 0) {
            return 0;
        } else {
            return notes[len - 1];
        }
    }

    int getInstrumentId(const stringPool::stringPtr& n);
    std::tuple<int, int, bool> getInstrumentTrack(const char* name);
    void loadInstrument(int id);
    inline void loadInstrument(const stringPool::stringPtr& n) {
        loadInstrument(getInstrumentId(n));
    }

   private:
    float noteAreaHeight;
    float noteAreaWidth;
    float realLookAtY;
    std::unordered_map<std::string, int> instrument2Id;
    void instrument2Id_init();
    bool instrumentLoaded[128];

   public:
    std::vector<displayBuffer_t> displayBuffer;
    clipboard_t* clipboard;
    void undo();
    void redo();
    void copy();
    bool pasteMode = false;
    bool editStatus = false;
    inline bool isEdited() {
        return editStatus;
    }
    inline void pushHistory(std::shared_ptr<history>& ptr) {
        histories_undo.push_back(ptr);
        histories_redo.clear();
        editStatus = true;
        editStatusUpdate();
    }

   private:
    std::list<std::shared_ptr<history>> histories_undo;
    std::list<std::shared_ptr<history>> histories_redo;
};
}  // namespace mgnr
#endif

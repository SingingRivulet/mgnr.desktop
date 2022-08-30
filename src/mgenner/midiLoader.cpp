#include <iostream>
#include <map>
#include <string>
#include "MidiFile.h"
#include "Options.h"
#include "editTable.h"
extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}
namespace mgnr {

using namespace std;
using namespace smf;

const char* instrumentName[128] = {
    "Piano",
    "BrightPiano",
    "ElectricPiano",
    "HonkyTonkPiano",
    "RhodesPiano",
    "ChorusedPiano",
    "Harpsichord",
    "Clavinet",
    "Celesta",
    "Glockenspiel",
    "MusicBoX",
    "Vibraphone",
    "Marimba",
    "Xylophone",
    "TubularBells",
    "Dulcimer",
    "HammondOrgan",
    "PercussiveOrgan",
    "RockOrgan",
    "ChurchOrgan",
    "ReedOrgan",
    "Accordion",
    "Harmonica",
    "TangoAccordian",
    "Guitar-nylon",
    "Guitar-steel",
    "Guitar-jazz",
    "Guitar-clean",
    "Guitar-muted",
    "OverdrivenGuitar",
    "DistortionGuitar",
    "GuitarHarmonics",
    "AcousticBass",
    "ElectricBass-finger",
    "ElectricBass-pick",
    "FretlessBass",
    "SlapBass1",
    "SlapBass2",
    "SynthBass1",
    "SynthBass2",
    "Violin",
    "Viola",
    "Cello",
    "Contrabass",
    "TremoloStrings",
    "PizzicatoStrings",
    "OrchestralHarp",
    "Timpani",
    "StringEnsemble1",
    "StringEnsemble2",
    "SynthStrings1",
    "SynthStrings2",
    "ChoirAahs",
    "VoiceOohs",
    "SynthVoice",
    "OrchestraHit",
    "Trumpet",
    "Trombone",
    "Tuba",
    "MutedTrumpet",
    "FrenchHorn",
    "BrassSection",
    "SynthBrass1",
    "SynthBrass2",
    "SopranoSaX",
    "AltoSaX",
    "TenorSaX",
    "BaritoneSaX",
    "Oboe",
    "EnglishHorn",
    "Bassoon",
    "Clarinet",
    "Piccolo",
    "Flute",
    "Record",
    "PanFlute",
    "BottleBlow",
    "Skakuhachi",
    "Whistle",
    "Ocarina",
    "Lead1-square",
    "Lead2-sawtooth",
    "Lead3-calliope",
    "Lead4-chiff",
    "Lead5-charang",
    "Lead6-voice",
    "Lead7-fifths",
    "Lead8-bass",
    "Pad1-newage",
    "Pad2-warm",
    "Pad3-polysynth",
    "Pad4-choir",
    "Pad5-bowed",
    "Pad6-metallic",
    "Pad7-halo",
    "Pad8-sweep",
    "FX1-rain",
    "FX2-soundtrack",
    "FX3-crystal",
    "FX4-atmosphere",
    "FX5-brightness",
    "FX6-goblins",
    "FX7-echoes",
    "FX8-sci-fi",
    "Sitar",
    "Banjo",
    "Shamisen",
    "Koto",
    "Kalimba",
    "Bagpipe",
    "Fiddle",
    "Shanai",
    "Tinkle Bell",
    "Agogo",
    "SteelDrums",
    "Woodblock",
    "TaikoDrum",
    "MelodicTom",
    "SynthDrum",
    "ReverseCymbal",
    "GuitarFretNoise",
    "BreathNoise",
    "Seashore",
    "BirdTweet",
    "TelephoneRing",
    "Helicopter",
    "Applause",
    "Gunshot"};

void editTable::addNoteWithId(float position, float tone, float dur, int v, int ins) {
    char infoBuf[128];
    int instrumentId = ins;
    if (instrumentId < 0 || instrumentId >= 128) {
        instrumentId = 0;
    }
    snprintf(infoBuf, sizeof(infoBuf), "%s.1", instrumentName[instrumentId]);
    midiMap::addNote(position, tone, dur, v, strPool.create(infoBuf));
}

void editTable::instrument2Id_init() {
    for (int i = 0; i < 128; ++i) {
        instrument2Id[instrumentName[i]] = i;
    }
}
void editTable::loadInstrument(int id) {
    if (id < 0 || id >= 128)
        return;
    if (instrumentLoaded[id])
        return;
    instrumentLoaded[id] = true;

    printf("mgenner:require instrument:%s\n", instrumentName[id]);
}
int editTable::getInstrumentId(const stringPool::stringPtr& name) {
    char n[128];
    snprintf(n, 128, "%s", name.c_str());
    for (int i = 0; i < 128; ++i) {
        if (n[i] == '\0') {
            break;
        } else if (n[i] == '.') {
            n[i] = '\0';
            break;
        }
    }
    auto it = instrument2Id.find(n);
    if (it == instrument2Id.end()) {
        return 0;
    } else {
        return it->second;
    }
}
std::tuple<int, int, bool> editTable::getInstrumentTrack(const char* name) {
    char n[128];
    bzero(n, 128);
    snprintf(n, 128, "%s", name);
    int i;
    for (i = 0; i < 128; ++i) {
        if (n[i] == '\0') {
            break;
        } else if (n[i] == '.') {
            n[i] = '\0';
            break;
        }
    }
    ++i;

    bool haveTrack = true;
    int t = 0;
    if (i >= 128 || n[i] == '\0') {
        haveTrack = false;
    } else {
        if (sscanf(&n[i], "%d", &t) < 1) {
            haveTrack = false;
        }
        if (t > 1024) {
            t = 1024;
        }
        if (t < 0) {
            t = 0;
        }
    }

    auto it = instrument2Id.find(n);
    if (it == instrument2Id.end()) {
        return std::tuple<int, int, bool>(t, -1, haveTrack);
    }
    return std::tuple<int, int, bool>(t, it->second, haveTrack);
}

void editTable::resetTrackMapper() {
    trackNameMapper.clear();
    trackInsMapper.clear();
    checkTrackMapper(trackNameMapper, trackInsMapper);
}
#define HAVE_TRACK 2
#define INSTRUMENT_ID 1
#define TRACK_ID 0
bool editTable::checkTrackMapper(std::map<std::string, int>& trackNameMapper,
                                 std::map<int, int>& trackInsMapper) {
    bool res = true;
    int maxTrack = -1;
    std::unordered_set<std::string> tracks;
    for (auto it : notes) {
        tracks.insert(it->info.value());
    }
    std::vector<std::string> mapperArr;
    for (auto& it : tracks) {
        if (trackNameMapper.find(it) == trackNameMapper.end()) {
            auto t = getInstrumentTrack(it.c_str());
            //printf("%s %d %d\n", it->info.c_str(), std::get<0>(t), std::get<1>(t));
            if (std::get<HAVE_TRACK>(t)) {
                //有音轨
                trackNameMapper[it] = std::get<TRACK_ID>(t);
                if (std::get<TRACK_ID>(t) > maxTrack) {
                    maxTrack = std::get<TRACK_ID>(t);
                }
            } else {
                //自动分配音轨
                res = false;
                mapperArr.push_back(it);
            }
        }
    }
    //分配音轨
    int i = maxTrack + 1;
    for (auto& it : mapperArr) {
        trackNameMapper[it] = i;
        ++i;
    }
    //检查乐器
    for (auto& it : trackNameMapper) {
        if (trackInsMapper.find(it.second) == trackInsMapper.end()) {
            res = false;
            auto t = getInstrumentTrack(it.first.c_str());
            if (std::get<INSTRUMENT_ID>(t) >= 0) {
                trackInsMapper[it.second] = std::get<INSTRUMENT_ID>(t);
            }
        }
    }
    for (auto& it : trackNameMapper) {
        if (trackInsMapper.find(it.second) == trackInsMapper.end()) {
            res = false;
            auto t = getInstrumentTrack(it.first.c_str());
            if (std::get<INSTRUMENT_ID>(t) < 0) {
                trackInsMapper[it.second] = 0;
                printf("mgenner:checkTrackMapper:track no instrument:%d\n", it.second);
                res = false;
            } else {
                trackInsMapper[it.second] = std::get<INSTRUMENT_ID>(t);
            }
        }
    }
    return res;
}

void editTable::loadMidi(const std::string& str) {
    MidiFile midifile;
    midifile.read(str);
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();

    TPQ = midifile.getTicksPerQuarterNote();
    rebuildNoteLen();
    printf("mgenner:TPQ:%d\n", TPQ);
    int tracks = midifile.getTrackCount();

    if (tracks > 1) {
        printf("mgenner:TRACKS:%d\n", tracks);
    }

    std::set<int> iset;

    for (int track = 0; track < tracks; track++) {
        char infoBuf[128];

        int instrumentId = 0;

        auto& track_obj = midifile[track];

        for (int event = 0; event < track_obj.size(); event++) {
            auto& event_obj = track_obj[event];

            if (event_obj.isNoteOn() && event_obj.size() >= 3) {
                int position = event_obj.tick;
                int delay = event_obj.getTickDuration();
                int delayS = event_obj.getDurationInSeconds();
                int tone = (int)event_obj[1];
                int v = (int)event_obj[2];
                snprintf(infoBuf, sizeof(infoBuf), "%s.%d", instrumentName[instrumentId], track);

                trackInsMapper[track] = instrumentId;
                trackNameMapper[infoBuf] = track;

                addNote(position, tone, delay, v, strPool.create(infoBuf));
                iset.insert(instrumentId);
            } else if (event_obj.isTimbre()) {
                instrumentId = event_obj.getP1();
                if (instrumentId < 0)
                    instrumentId = 0;
                else if (instrumentId > 128)
                    instrumentId = 128;
            } else if (event_obj.isMetaMessage()) {
                if (event_obj.isMarkerText()) {
                    int position = event_obj.tick;
                    snprintf(infoBuf, sizeof(infoBuf), "%s.%d", instrumentName[instrumentId],
                             track);

                    trackInsMapper[track] = instrumentId;
                    trackNameMapper[infoBuf] = track;

                    addDescription(strPool.create(infoBuf), position, event_obj.getMetaContent());
                }
            }
        }
    }

    auto numTracks = midifile.getNumTracks();
    printf("mgenner:load controls numTracks:%d\n", numTracks);
    for (int trackIndex = 0; trackIndex < numTracks; ++trackIndex) {
        for (int i = 0; i < midifile.getNumEvents(trackIndex); i++) {
            if (midifile.getEvent(trackIndex, i).isTempo()) {  //是设置时间
                double tp = midifile.getEvent(trackIndex, i).getTempoBPM();
                addTempo(midifile.getEvent(trackIndex, i).tick, tp);
            }
        }
    }

    printf("mgenner:load instruments\n");
    for (auto it : iset) {
        loadInstrument(it);
    }

    printf("mgenner:load midi success\n");
}

void editTable::onScriptCmd(const char*) {}

std::string editTable::loadMidi_preprocess(const std::string& str, const std::string& script, int basetone) {
    MidiFile midifile;
    midifile.read(str);
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();

    auto lua = luaL_newstate();
    luaL_openlibs(lua);
    if (luaL_loadstring(lua, script.c_str()) || lua_pcall(lua, 0, 0, 0)) {
        //载入脚本
        std::string res = lua_tostring(lua, -1);
        printf("mgenner:script:%s\n", res.c_str());
        lua_close(lua);
        return res;
    }

    lua_pushcfunction(lua, [](lua_State* L) -> int {
        //self, num
        if (!lua_islightuserdata(L, 1)) {
            return 0;
        }
        auto self = (editTable*)lua_touserdata(L, 1);
        if (self == nullptr) {
            return 0;
        }
        int num = luaL_checkinteger(L, 2);
        self->pitchNum = num;
        return 0;
    });
    lua_setglobal(lua, "setPitchNum");

    lua_pushcfunction(lua, [](lua_State* L) -> int {
        //self, cmd
        if (!lua_islightuserdata(L, 1)) {
            return 0;
        }
        auto self = (editTable*)lua_touserdata(L, 1);
        if (self == nullptr) {
            return 0;
        }
        const char* cmd = luaL_checkstring(L, 2);
        self->onScriptCmd(cmd);
        return 0;
    });
    lua_setglobal(lua, "cmd");

    lua_pushcfunction(lua, [](lua_State* L) -> int {
        const char* str = luaL_checkstring(L, 1);
        printf("mgenner:script_log:%s\n", str);
        return 0;
    });
    lua_setglobal(lua, "log_print");

    lua_pushcfunction(lua, [](lua_State* L) -> int {
        //self, position, tone, delay, v,infoBuf
        if (!lua_islightuserdata(L, 1)) {
            return 0;
        }
        auto self = (editTable*)lua_touserdata(L, 1);
        if (self == nullptr) {
            return 0;
        }
        int position = luaL_checkinteger(L, 2);
        int tone = luaL_checkinteger(L, 3);
        int dur = luaL_checkinteger(L, 4);
        int v = luaL_checkinteger(L, 5);
        const char* info = luaL_checkstring(L, 6);
        lua_pushinteger(L, self->addNote(position, tone, dur, v, self->strPool.create(info))->id);
        return 1;
    });
    lua_setglobal(lua, "addNote");

    //取得函数
    lua_getglobal(lua, "process");
    if (!lua_isfunction(lua, -1)) {
        printf("mgenner:function 'process' no found\n");
        lua_close(lua);
        return "function 'process' no found";
    }

    lua_pushlightuserdata(lua, this);
    lua_createtable(lua, 0, 0);
    int index = 0;

    TPQ = midifile.getTicksPerQuarterNote();
    rebuildNoteLen();
    printf("mgenner:TPQ:%d\n", TPQ);
    int tracks = midifile.getTrackCount();

    if (tracks > 1) {
        printf("mgenner:TRACKS:%d\n", tracks);
    }

    std::set<int> iset;

    std::string res;

    for (int track = 0; track < tracks; track++) {
        char infoBuf[128];

        int instrumentId = 0;

        auto& track_obj = midifile[track];

        for (int event = 0; event < track_obj.size(); event++) {
            auto& event_obj = track_obj[event];
            if (event_obj.isNoteOn() && event_obj.size() >= 3) {
                int position = event_obj.tick;
                int delay = event_obj.getTickDuration();
                double delayS = event_obj.getDurationInSeconds();
                int tone = (int)event_obj[1];
                int v = (int)event_obj[2];
                snprintf(infoBuf, sizeof(infoBuf), "%s.%d", instrumentName[instrumentId], track);
                //addNote(position, tone, delay, v,infoBuf);

                lua_createtable(lua, 0, 0);
                //创建lua数组

                lua_pushstring(lua, "time");
                lua_pushinteger(lua, position);
                lua_settable(lua, -3);

                lua_pushstring(lua, "duration");
                lua_pushinteger(lua, delay);
                lua_settable(lua, -3);

                lua_pushstring(lua, "durationInSeconds");
                lua_pushnumber(lua, delayS);
                lua_settable(lua, -3);

                lua_pushstring(lua, "tone");
                lua_pushinteger(lua, tone);
                lua_settable(lua, -3);

                lua_pushstring(lua, "instrument");
                lua_pushinteger(lua, instrumentId);
                lua_settable(lua, -3);

                lua_pushstring(lua, "info");
                lua_pushstring(lua, infoBuf);
                lua_settable(lua, -3);

                lua_pushstring(lua, "volume");
                lua_pushinteger(lua, v);
                lua_settable(lua, -3);

                lua_rawseti(lua, -2, ++index);

                iset.insert(instrumentId);
            } else if (event_obj.isTimbre()) {
                instrumentId = event_obj.getP1();
                if (instrumentId < 0)
                    instrumentId = 0;
                else if (instrumentId > 128)
                    instrumentId = 128;
            }
        }
    }

    lua_pushinteger(lua, basetone);

    if (lua_pcall(lua, 3, 0, 0)) {
        res = lua_tostring(lua, -1);
        printf("mgenner:script:%s\n", res.c_str());
    }

    auto numTracks = midifile.getNumTracks();
    printf("mgenner:load controls numTracks:%d\n", numTracks);
    for (int trackIndex = 0; trackIndex < numTracks; ++trackIndex) {
        for (int i = 0; i < midifile.getNumEvents(trackIndex); i++) {
            if (midifile.getEvent(trackIndex, i).isTempo()) {  //是设置时间
                double tp = midifile.getEvent(trackIndex, i).getTempoBPM();
                addTempo(midifile.getEvent(trackIndex, i).tick, tp);
            }
        }
    }

    printf("mgenner:load instruments\n");
    for (auto it : iset) {
        loadInstrument(it);
    }

    lua_close(lua);

    printf("mgenner:load midi success\n");

    return res;
}

std::string editTable::exportString() {
    struct noteMap_t {
        int tone, volume, time;
        bool isNoteOn;
        int instrument;
    };

    std::string res;
    std::vector<noteMap_t*> noteMap;

    for (auto it : notes) {
        if (it->info.empty()) {
        } else {
            if (it->info.at(0) != '@') {  //为@是控制字符
                auto p1 = new noteMap_t;
                p1->tone = it->tone;
                p1->volume = it->volume > 100 ? 100 : it->volume;
                p1->time = it->begin;
                p1->isNoteOn = true;

                auto p2 = new noteMap_t;
                p2->tone = it->tone;
                p2->volume = 0;
                p2->time = it->begin + it->delay;
                p2->isNoteOn = false;

                p1->instrument = getInstrumentId(it->info);
                p2->instrument = p1->instrument;
                noteMap.push_back(p1);
                noteMap.push_back(p2);
            }
        }
    }
    sort(noteMap.begin(), noteMap.end(), [](noteMap_t* a, noteMap_t* b) {
        if (a->time < b->time) {
            return true;
        }
        if (a->time == b->time && b->isNoteOn && !a->isNoteOn) {
            return true;
        }
        return false;
    });
    char buf[256];
    for (auto& it : noteMap) {
        bzero(buf, sizeof(buf));
        if (it->isNoteOn) {
            snprintf(buf, sizeof(buf), "B %d %d %d %d\n", it->time, it->tone, it->instrument, it->volume);
        } else {
            snprintf(buf, sizeof(buf), "E %d %d %d\n", it->time, it->tone, it->instrument);
        }
        res += buf;
        delete it;
    }
    return res;
}

void editTable::exportMidi(const std::string& filename) {
    map<stringPool::stringPtr, int> tracks;
    int trackNum = 1;  //0音轨存没有info的音符
    int track;
    MidiFile midifile;

    midifile.setTPQ(TPQ);  //0音轨
    midifile.addTrack();   //0音轨

    struct noteMap_t {
        int tone, volume, time;
        bool isNoteOn;
        bool isMarker;
        std::string markerMessage{};
    };

    std::map<int, std::pair<int, std::vector<noteMap_t*>>> noteMap;

    //添加音符
    for (auto it : notes) {
        if (it->info.empty()) {
            track = 0;

        } else {
            if (it->info.at(0) != '@') {  //为@是控制字符
                auto tit = tracks.find(it->info);

                if (tit == tracks.end()) {  //没有音轨
                    midifile.addTrack();
                    tracks[it->info] = trackNum;
                    track = trackNum;
                    ++trackNum;

                    auto p1 = new noteMap_t;
                    p1->tone = it->tone;
                    p1->volume = it->volume > 100 ? 100 : it->volume;
                    p1->time = it->begin;
                    p1->isNoteOn = true;
                    p1->isMarker = false;

                    auto p2 = new noteMap_t;
                    p2->tone = it->tone;
                    p2->volume = 0;
                    p2->time = it->begin + it->delay;
                    p2->isNoteOn = false;
                    p2->isMarker = false;

                    auto& lst = noteMap[track];
                    lst.first = getInstrumentId(it->info);
                    lst.second.push_back(p1);
                    lst.second.push_back(p2);

                } else {
                    track = tit->second;

                    auto p1 = new noteMap_t;
                    p1->tone = it->tone;
                    p1->volume = it->volume > 100 ? 100 : it->volume;
                    p1->time = it->begin;
                    p1->isNoteOn = true;
                    p1->isMarker = false;

                    auto p2 = new noteMap_t;
                    p2->tone = it->tone;
                    p2->volume = 0;
                    p2->time = it->begin + it->delay;
                    p2->isNoteOn = false;
                    p2->isMarker = false;

                    auto& lst = noteMap[track];
                    lst.second.push_back(p1);
                    lst.second.push_back(p2);
                }
            }
        }
    }
    //添加文本信息
    for (auto& it_title : descriptions) {
        auto tit = tracks.find(it_title.first);
        int ins = getInstrumentId(it_title.first);

        if (tit == tracks.end()) {  //没有音轨
            midifile.addTrack();
            tracks[it_title.first] = trackNum;
            track = trackNum;
            ++trackNum;

            for (auto& it_content : it_title.second) {
                if (it_content.first >= 0) {
                    auto p = new noteMap_t;
                    p->time = it_content.first;
                    p->isNoteOn = false;
                    p->isMarker = true;
                    p->markerMessage = it_content.second;

                    auto& lst = noteMap[track];
                    lst.first = ins;
                    lst.second.push_back(p);
                }
            }

        } else {
            track = tit->second;

            for (auto& it_content : it_title.second) {
                if (it_content.first >= 0) {
                    auto p = new noteMap_t;
                    p->time = it_content.first;
                    p->isNoteOn = false;
                    p->isMarker = true;
                    p->markerMessage = it_content.second;

                    auto& lst = noteMap[track];
                    lst.first = ins;
                    lst.second.push_back(p);
                }
            }
        }
    }

    for (auto it : timeMap) {  //添加time map
        midifile.addTempo(0, it.first, it.second);
    }
    for (auto itlst : noteMap) {
        int tk = itlst.first;
        int ch = tk;
        if (ch > 15)
            ch = 15;

        sort(itlst.second.second.begin(), itlst.second.second.end(),
             [](noteMap_t* a, noteMap_t* b) {
                 if (a->time < b->time) {
                     return true;
                 }
                 if (a->time == b->time && b->isNoteOn && !a->isNoteOn) {
                     return true;
                 }
                 return false;
             });

        midifile.addTimbre(tk, 0, ch, itlst.second.first);

        for (auto it : itlst.second.second) {  //扫描音轨
            if (it->isMarker) {
                midifile.addMarker(tk, it->time, it->markerMessage);
            } else {
                if (it->isNoteOn) {
                    midifile.addNoteOn(tk, it->time, ch, it->tone, it->volume);
                } else {
                    midifile.addNoteOff(tk, it->time, ch, it->tone);
                }
            }
            delete it;
        }
    }
    midifile.write(filename);
    editStatus = false;
    editStatusUpdate();
}

void editTable::exportMidiWithTrackMapper(const std::string& filename) {
    int maxTrack = 0;
    for (auto& it : trackNameMapper) {
        if (it.second > maxTrack) {
            maxTrack = it.second;
        }
    }
    ++maxTrack;
    MidiFile midifile;

    midifile.setTPQ(TPQ);
    for (int i = 0; i < maxTrack; ++i) {
        midifile.addTrack();
    }

    struct noteMap_t {
        int tone, volume, time;
        int track, ins, channel = -1;
        noteMap_t* pair = nullptr;
        bool isNoteOn;
        bool isMarker;
        std::string markerMessage{};
    };

    std::vector<noteMap_t*> noteMap;

    //添加音符
    for (auto it : notes) {
        if (!it->info.empty() && it->info.at(0) != '@') {
            auto tit = trackNameMapper.find(it->info.value());
            if (tit != trackNameMapper.end()) {
                int track = tit->second;
                int ins = 0;
                auto ins_it = trackInsMapper.find(track);
                if (ins_it != trackInsMapper.end()) {
                    ins = ins_it->second;
                }

                auto p1 = new noteMap_t;
                p1->tone = it->tone;
                p1->volume = it->volume > 100 ? 100 : it->volume;
                p1->time = it->begin;
                p1->isNoteOn = true;
                p1->isMarker = false;
                p1->ins = ins;
                p1->track = track;

                auto p2 = new noteMap_t;
                p2->tone = it->tone;
                p2->volume = 0;
                p2->time = it->begin + it->delay;
                p2->isNoteOn = false;
                p2->isMarker = false;
                p2->ins = ins;
                p2->track = track;

                p1->pair = p2;
                p2->pair = p1;

                noteMap.push_back(p1);
                noteMap.push_back(p2);
            }
        }
    }
    //添加文本信息
    for (auto& it_title : descriptions) {
        auto tit = trackNameMapper.find(it_title.first.value());
        if (tit != trackNameMapper.end()) {
            int track = tit->second;
            int ins = 0;
            auto ins_it = trackInsMapper.find(track);
            if (ins_it != trackInsMapper.end()) {
                ins = ins_it->second;
            }

            for (auto& it_content : it_title.second) {
                if (it_content.first >= 0) {
                    auto p = new noteMap_t;
                    p->time = it_content.first;
                    p->isNoteOn = false;
                    p->isMarker = true;
                    p->markerMessage = it_content.second;
                    p->ins = ins;
                    p->track = track;

                    noteMap.push_back(p);
                }
            }
        }
    }

    for (auto it : timeMap) {  //添加time map
        midifile.addTempo(0, it.first, it.second);
    }
    sort(noteMap.begin(), noteMap.end(),
         [](noteMap_t* a, noteMap_t* b) {
             if (a->time < b->time) {
                 return true;
             }
             if (a->time == b->time && b->isNoteOn && !a->isNoteOn) {
                 return true;
             }
             return false;
         });

    std::map<int, int> ins2Channel;
    std::tuple<std::set<int>, int> channelUsing[16]{};
    for (int i = 0; i < 16; ++i) {
        std::get<1>(channelUsing[i]) = -1;
    }

    for (auto it : noteMap) {
        if (it->isMarker) {
            midifile.addMarker(it->track, it->time, it->markerMessage);
        } else {
            if (it->isNoteOn) {
                //规划通道
                auto it_ins = ins2Channel.find(it->ins);
                if (it_ins == ins2Channel.end()) {
                    //找未使用的通道
                    for (int i = 0; i < 16; ++i) {
                        if (std::get<1>(channelUsing[i]) < 0 &&
                            std::get<0>(channelUsing[i]).empty()) {
                            ins2Channel[it->ins] = i;
                            std::get<1>(channelUsing[i]) = it->ins;
                            std::get<0>(channelUsing[i]).insert(it->tone);
                            it->channel = i;
                            it->pair->channel = i;
                            midifile.addTimbre(it->track, it->time, i, it->ins);
                            goto addNoteOn;
                        }
                    }
                    //找一个空通道
                    for (int i = 0; i < 16; ++i) {
                        if (std::get<0>(channelUsing[i]).empty()) {
                            if (std::get<1>(channelUsing[i]) >= 0) {
                                //关闭之前的音符
                                ins2Channel.erase(std::get<1>(channelUsing[i]));
                            }
                            ins2Channel[it->ins] = i;
                            std::get<1>(channelUsing[i]) = it->ins;
                            std::get<0>(channelUsing[i]).insert(it->tone);
                            it->channel = i;
                            it->pair->channel = i;
                            midifile.addTimbre(it->track, it->time, i, it->ins);
                            goto addNoteOn;
                        }
                    }
                    //没有通道可用
                    goto playNoteFail;
                } else {
                    std::get<0>(channelUsing[it_ins->second]).insert(it->tone);
                    it->channel = it_ins->second;
                    it->pair->channel = it_ins->second;
                }
            addNoteOn:
                midifile.addNoteOn(it->track, it->time, it->channel, it->tone, it->volume);
            } else {
                if (it->channel >= 0 && it->channel < 16 &&
                    std::get<1>(channelUsing[it->channel]) == it->ins) {
                    std::get<0>(channelUsing[it->channel]).erase(it->tone);
                    midifile.addNoteOff(it->track, it->time, it->channel, it->tone);
                }
            }
        }
    playNoteFail:
        delete it;
    }
    midifile.write(filename);
    editStatus = false;
    editStatusUpdate();
}

}  // namespace mgnr

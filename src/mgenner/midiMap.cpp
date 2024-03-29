#include "midiMap.h"
#include "mempool.h"
namespace mgnr {

typedef mempool<note> npool;

midiMap::midiMap() {
    pool = new npool;
    XShift = 0;
    noteTimeMax = 0;
    noteUpdated = true;

    chord_map["maj3"] = {0, 4, 7, 0};    // 大三和弦 根音-大三度-纯五度
    chord_map["min3"] = {0, 3, 7, 0};    //小三和弦 根音-小三度-纯五度
    chord_map["aug3"] = {0, 4, 8, 0};    //增三和弦 根音-大三度-增五度
    chord_map["dim3"] = {0, 3, 6, 0};    //减三和弦 根音-小三度-减五度
    chord_map["M7"] = {0, 4, 7, 11};     //大七和弦 根音-大三度-纯五度-大七度
    chord_map["Mm7"] = {0, 4, 7, 10};    //属七和弦 根音-大三度-纯五度-小七度
    chord_map["m7"] = {0, 3, 7, 10};     //小七和弦 根音-小三度-纯五度-小七度
    chord_map["mM7"] = {0, 3, 7, 11};    //小大七和弦 根音-小三度-纯五度-大七度
    chord_map["aug7"] = {0, 4, 8, 10};   //增七和弦 根音-大三度-增五度-小七度
    chord_map["augM7"] = {0, 4, 8, 11};  //增大七和弦 根音-大三度-增五度-小七度
    chord_map["m7b5"] = {0, 3, 6, 10};   //半减七和弦 根音-小三度-减五度-减七度
    chord_map["dim7"] = {0, 3, 6, 9};    //减减七和弦 根音-小三度-减五度-减七度
    note_number_map["1"] = 0;
    note_number_map["1#"] = 1;
    note_number_map["2"] = 2;
    note_number_map["2#"] = 3;
    note_number_map["3"] = 4;
    note_number_map["4"] = 5;
    note_number_map["4#"] = 6;
    note_number_map["5"] = 7;
    note_number_map["5#"] = 8;
    note_number_map["6"] = 9;
    note_number_map["6#"] = 10;
    note_number_map["7"] = 11;
    int n = 60;
    chord_map_note["C"] = n;
    n += 2;
    chord_map_note["D"] = n;
    n += 2;
    chord_map_note["E"] = n;
    n += 1;
    chord_map_note["F"] = n;
    n += 2;
    chord_map_note["G"] = n;
    n += 2;
    chord_map_note["A"] = n;
    n += 2;
    chord_map_note["B"] = n;
}
midiMap::~midiMap() {
    clear();
    if (pool)
        delete (npool*)pool;
}
bool midiMap::updateTimeMax() {
    if (noteUpdated) {
        noteTimeMax = 0;
        noteToneMin = 255;
        noteToneMax = 0;
        for (auto& it : notes) {
            float tm = it->begin + it->duration;
            if (tm > noteTimeMax) {
                noteTimeMax = tm;
            }
            float tone = it->tone;
            if (tone > 0 && tone < 128) {
                if (tone > noteToneMax) {
                    noteToneMax = tone;
                }
                if (tone < noteToneMin) {
                    noteToneMin = tone;
                }
            }
        }
        noteUpdated = false;
        return true;
    }
    return false;
}
note* midiMap::addNote(float position, float tone, float duration, int v, const stringPool::stringPtr& info, int id_set) {
    onUseInfo(info);

    HBB::vec from;
    HBB::vec to;
    from.set(position, tone);
    to = from;
    to.X += duration;
    to.Y += 0.9;

    auto p = ((npool*)pool)->get();
    p->construct();
    if (id_set > 0 && noteIDs.find(id_set) == noteIDs.end()) {
        p->id = id_set;
    } else {
        p->id = ++id;
    }
    p->startId = ++id;  //0是默认值，用0可能会出问题
    p->endId = ++id;    //分配两个id，防止时间为0的情况

    p->begin = position;
    p->tone = tone;
    p->duration = duration;
    p->volume = v;
    p->info = info;

    p->getBeginIndex();
    p->getEndIndex();

    noteIDs[p->id] = p;
    timeIndex[p->beginIndex] = p;
    timeIndex[p->endIndex] = p;

    auto bx = indexer.add(from, to, p);
    p->indexer = bx;

    notes.insert(p);

    if (!info.empty()) {
        if (info[0] == '@') {
            addControl(position, info);
        }
    }

    noteUpdated = true;
    return p;
}

void midiMap::resizeNote(note* p) {
    if (p) {
        if (p->indexer) {
            auto i = (HBB::AABB*)(p->indexer);
            i->autodrop();
        }

        HBB::vec from;
        HBB::vec to;
        from.set(p->begin, p->tone);
        to = from;
        to.X += p->duration;
        to.Y += 0.9;

        timeIndex.erase(p->endIndex);
        p->getEndIndex();  //更新尾部索引
        timeIndex[p->endIndex] = p;

        auto bx = indexer.add(from, to, p);
        p->indexer = bx;

        noteUpdated = true;
    }
}

void midiMap::moveNote(note* p) {
    if (p) {
        if (p->indexer) {
            auto i = (HBB::AABB*)(p->indexer);
            i->autodrop();
        }

        HBB::vec from;
        HBB::vec to;
        from.set(p->begin, p->tone);
        to = from;
        to.X += p->duration;
        to.Y += 0.9;

        timeIndex.erase(p->beginIndex);
        timeIndex.erase(p->endIndex);
        p->getBeginIndex();  //更新头部索引
        p->getEndIndex();    //更新尾部索引
        timeIndex[p->beginIndex] = p;
        timeIndex[p->endIndex] = p;

        auto bx = indexer.add(from, to, p);
        p->indexer = bx;

        noteUpdated = true;
    }
}

void midiMap::removeNote(note* p, bool unselect) {
    if (p) {
        if (p->indexer) {
            auto i = (HBB::AABB*)(p->indexer);
            i->autodrop();
        }
        if (!p->info.empty()) {
            if (p->info[0] == '@') {
                removeControl(p->begin, p->info);
            }
        }
        notes.erase(p);
        if (unselect && p->selected) {
            selected.erase(p);
            updateSelectedStatus();
        }
        noteIDs.erase(p->id);
        timeIndex.erase(p->beginIndex);
        timeIndex.erase(p->endIndex);
        *p = note();
        ((npool*)pool)->del(p);

        noteUpdated = true;
    }
}

void midiMap::clear() {
    for (auto p : notes) {
        if (p->indexer) {
            auto i = (HBB::AABB*)(p->indexer);
            i->autodrop();
        }
        *p = note();
        ((npool*)pool)->del(p);
        //不能直接removeNote，否则会出现野指针
    }
    notes.clear();
    timeIndex.clear();
    noteUpdated = true;
}

int midiMap::find(const HBB::vec& from, const HBB::vec& to, void (*callback)(note*, void*), void* arg) {
    HBB::AABB tmpbox;
    tmpbox.from = from;
    tmpbox.to = to;

    struct self {
        void (*callback)(note*, void*);
        void* arg;
        midiMap* s;
        int num;
    } s;
    s.arg = arg;
    s.callback = callback;
    s.s = this;
    s.num = 0;

    indexer.collisionTest(
        &tmpbox, [](HBB::AABB* p, void* arg) {
            auto s = (self*)arg;
            auto np = (note*)(p->data);
            if (np && (s->s->infoFilter.empty() || np->info == s->s->infoFilter)) {
                s->callback(np, s->arg);
                s->num++;
            }
        },
        &s);
    return s.num;
}

int midiMap::find(const HBB::vec& pt, void (*callback)(note*, void*), void* arg) {
    struct self {
        void (*callback)(note*, void*);
        void* arg;
        midiMap* s;
        int num;
    } s;
    s.arg = arg;
    s.callback = callback;
    s.s = this;
    s.num = 0;

    indexer.fetchByPoint(
        pt, [](HBB::AABB* p, void* arg) {
            auto s = (self*)arg;
            auto np = (note*)(p->data);
            if (np && (s->s->infoFilter.empty() || np->info == s->s->infoFilter)) {
                s->callback(np, s->arg);
                s->num++;
            }
        },
        &s);
    return s.num;
}

int midiMap::find(float step, void (*callback)(note*, void*), void* arg) {
    struct self {
        void (*callback)(note*, void*);
        void* arg;
        midiMap* s;
        int num;
    } s;
    s.arg = arg;
    s.callback = callback;
    s.s = this;
    s.num = 0;

    indexer.fetchByStep(
        step, [](HBB::AABB* p, void* arg) {
            auto s = (self*)arg;
            auto np = (note*)(p->data);
            if (np && (s->s->infoFilter.empty() || np->info == s->s->infoFilter)) {
                s->callback(np, s->arg);
                s->num++;
            }
        },
        &s);
    return s.num;
}
double midiMap::getTempo(int tick) {
    if (timeMap.empty())
        return 120;
    auto it = timeMap.upper_bound(tick);  //获取大于tick的第一个元素
    if (it == timeMap.end()) {            //指向结尾
        it--;
        return it->second;  //返回最后一个元素
    }
    if (it == timeMap.begin()) {  //指向开头
        return it->second;
    }
    it--;  //向前移动一步
    return it->second;
}
bool midiMap::addTempo(int tick, double tp) {
    if (timeMap.find(tick) == timeMap.end()) {
        printf("mgenner:add tempo:%d %f\n", tick, tp);
        timeMap[tick] = tp;
        return true;
    } else {
        return false;
    }
}
void midiMap::getTempo(int begin, const std::function<bool(int, double)>& callback) {
    if (timeMap.empty()) {
        callback(0, 120);
        return;
    }
    auto it = timeMap.upper_bound(begin);  //获取大于tick的第一个元素
    if (it == timeMap.end()) {             //指向结尾
        it--;
        callback(it->first, it->second);
        return;
    }
    if (it == timeMap.begin()) {  //指向开头

    } else {
        it--;  //向前移动一步
    }
    while (it != timeMap.end()) {
        if (!callback(it->first, it->second))
            break;
        ++it;
    }
}
std::tuple<bool, int, double> midiMap::removeTempoBeforePos(int tick) {
    if (timeMap.empty())
        return std::make_tuple(false, 0, 0.0);
    auto it = timeMap.upper_bound(tick);  //获取大于tick的第一个元素
    if (it == timeMap.end()) {            //指向结尾
        it--;
        auto res = std::make_tuple(true, it->first, it->second);
        timeMap.erase(it);
        return res;
    }
    if (it == timeMap.begin()) {  //指向开头
        auto res = std::make_tuple(true, it->first, it->second);
        timeMap.erase(it);
        return res;
    }
    it--;  //向前移动一步
    auto res = std::make_tuple(true, it->first, it->second);
    timeMap.erase(it);
    return res;
}
void midiMap::removeControl(float begin, const stringPool::stringPtr& info) {
    if (!info.empty() && info[0] == '@' && info.size() > 2) {
        auto str = info.c_str();
        if (str[1] == 'T') {
            auto r = str + 2;

            if (strlen(r) > 0) {
                int tick = atoi(r);
                if (tick > 0) {
                    printf("mgenner:remove tempo:%f %d\n", begin, tick);
                    timeMap.erase(begin);
                }
            }
        }
    }
}
void midiMap::addControl(float begin, const stringPool::stringPtr& info) {
    if (!info.empty() && info[0] == '@' && info.size() > 2) {
        auto str = info.c_str();
        if (str[1] == 'T') {
            auto r = str + 2;

            if (strlen(r) > 0) {
                double tick = atof(r);
                if (tick > 0) {
                    printf("mgenner:add tempo:%f %f\n", begin, tick);
                    timeMap[begin] = tick;
                }
            }
        } else if (str[1] == 'S') {
            auto r = str + 2;

            if (strlen(r) > 0) {
                int sec = atoi(r);
                if (sec > 0 && sec < 8) {
                    printf("mgenner:set section:%d\n", sec);
                    setSection(sec);
                }
            }
        }
    }
}
int midiMap::getAreaNote(float begin, float len, const std::string& info, float forceLen, float minLen) {
    struct self_t {
        std::vector<std::tuple<float, float, int> > tones;
        float sum;
        float begin;
        std::string info;
    } self;
    self.sum = 0;
    self.info = info;
    self.begin = begin;
    find(
        HBB::vec(begin, 0), HBB::vec(begin + len, 128), [](note* n, void* arg) {  //获取
            auto self = (self_t*)arg;
            if (!self->info.empty()) {
                if (n->info != self->info) {
                    return;
                }
            }
            if (n->begin < self->begin) {
                float delta = n->begin - self->begin;
                float dur = n->duration + delta;
                if (dur >= 1) {
                    self->tones.push_back(std::make_tuple(n->begin - delta, dur, n->tone));
                    self->sum += dur;
                }
            } else {
                self->tones.push_back(std::make_tuple(n->begin, n->duration, n->tone));
                self->sum += n->duration;
            }
        },
        &self);

    if (self.tones.empty() || self.sum <= 0)
        return -1;

    std::map<int, float> tone_time;
    for (auto& it : self.tones) {
        tone_time[std::get<2>(it)] += std::get<1>(it) / self.sum;
    }

    for (auto& it : tone_time) {
        if (it.second > forceLen) {
            return it.first;
        }
    }

    for (auto& it : self.tones) {
        if (tone_time[std::get<2>(it)] >= minLen) {
            return std::get<2>(it);
        }
    }
    return -1;
}
int midiMap::getSectionNote(float sec, const std::string& info, float forceLen, float minLen) {
    float len = TPQ;
    float pos = len * (section * sec + XShift);
    for (int i = 0; i < section; ++i) {
        int res = getAreaNote(pos, len, info, forceLen, minLen);
        if (res != -1) {
            return res;
        }
    }
    return -1;
}

}  //namespace mgnr

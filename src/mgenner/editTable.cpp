#include "editTable.h"
namespace mgnr {

editTable::editTable() {
    baseTone = 0;
    automaticX = true;
    automaticY = true;
    lookAtX = 0;
    lookAtY = 64;
    noteHeight = 20;
    noteLength = 0.6;
    defaultDelay = 120;
    maticBlock = 120;
    defaultVolume = 50;
    defaultInfo = strPool.create("default");
    selected.clear();
    instrument2Id_init();
    for (int i = 0; i < 128; ++i) {
        instrumentLoaded[i] = false;
    }
}

editTable::~editTable() {
}

void editTable::render() {
    drawNote_begin();
    findNote();
    drawDisplay();
    drawTempoLine();
    drawDescriptionsLine();
    drawScroll();
    drawCaptions();
    drawNote_end();
}

void editTable::drawDisplay() {
    for (auto& it : displayBuffer) {
        drawNoteAbs(
            it.begin,
            it.tone,
            it.dur,
            it.volume,
            it.info,
            false,
            true);
    }
}

HBB::vec editTable::screenToAbs(int x, int y) {
    //由于使用了三个私有变量，此操作必须在render后进行，否则结果将不准确
    float absy = ((float)(windowHeight - y)) / noteHeight + realLookAtY;  //先上下对称处理，然后除以高度，再加上左上角坐标
    float absx = ((float)x) / noteLength + lookAtX;                       //除以长度，加上左上角坐标
    HBB::vec res;
    res.X = absx;
    res.Y = absy;
    return res;
}

void editTable::automatic(float& x, float& y) {
    if (automaticX) {
        if (maticBlock > 0) {
            int num = floor(x / maticBlock);
            float real = num * maticBlock;
            x = real;
        }
    }
    if (automaticY) {
        y = (int)floor(y);
    }
}
void editTable::clickToLookAt(int x, int y) {
    updateTimeMax();  //更新进度条长度
    int m = noteTimeMax;
    if (m > 0) {
        lookAtX = (x * m) / windowWidth;
    } else {
        lookAtX = 0;
    }
}
void editTable::clickToRemoveTempo(int x, int y) {
    auto p = screenToAbs(x, y);
    automatic(p.X, p.Y);
    auto res = removeTempoBeforePos(p.X);
    if (std::get<0>(res)) {
        auto& tick = std::get<1>(res);
        auto& tempo = std::get<2>(res);
        printf("mgenner:remove tempo:%d=>%f\n", tick, tempo);
        auto hisptr = std::make_shared<history>();  //插入历史记录
        hisptr->method = history::H_TEMPO_DEL;
        hisptr->tempo = tempo;
        hisptr->begin = tick;
        pushHistory(hisptr);
    }
}
void editTable::clickToSetTempo(int x, int y, double tp) {
    auto p = screenToAbs(x, y);
    automatic(p.X, p.Y);
    if (tp > 0) {
        if (addTempo(p.X, tp)) {
            auto hisptr = std::make_shared<history>();  //插入历史记录
            hisptr->method = history::H_TEMPO_ADD;
            hisptr->tempo = tp;
            hisptr->begin = p.X;
            pushHistory(hisptr);
        }
    }
}

void editTable::drawCaptions() {
    auto it = captions.upper_bound(lookAtX);  //获取大于tick的第一个元素
    while (it != captions.end()) {
        auto p = (it->first - lookAtX) * noteLength;
        if (p > windowWidth) {
            break;
        } else {
            drawCaption(p, it->second);
        }
        ++it;
    }
}

void editTable::clickToAddDescription(int x, int y, const std::function<void(int tick)>& callback) {
    auto p = screenToAbs(x, y);
    automatic(p.X, p.Y);
    callback(p.X);
}
void editTable::clickToSetDescription(int x, int y, const std::function<void(int tick, const stringPool::stringPtr& title, const std::string& content)>& callback) {
    auto p = screenToAbs(x, y);
    automatic(p.X, p.Y);
    //查找左边最近的音符
    if (infoFilter.empty()) {  //多音轨显示模式
        int max_tick = 0;
        stringPool::stringPtr max_title;
        const std::string* max_content = nullptr;
        for (auto& it_filter : descriptions) {
            if (!it_filter.second.empty()) {
                auto it = it_filter.second.upper_bound(p.X);  //获取大于tick的第一个元素
                if (it != it_filter.second.begin()) {
                    //往前退
                    --it;
                    if (it != it_filter.second.end() && it->first > lookAtX) {
                        auto tick = it->first;
                        auto title = it_filter.first;
                        auto content = &it->second;
                        if (tick > max_tick || max_title == nullptr) {
                            max_tick = tick;
                            max_title = title;
                            max_content = content;
                        }
                    }
                }
            }
        }
        if (max_title.parent_node != nullptr && max_content != nullptr) {
            callback(max_tick, max_title, *max_content);
        }
    } else {  //单音轨
        auto it_filter = descriptions.find(infoFilter);
        if (it_filter != descriptions.end()) {
            if (!it_filter->second.empty()) {
                auto it = it_filter->second.upper_bound(p.X);  //获取大于tick的第一个元素
                if (it != it_filter->second.begin()) {
                    //往前退
                    --it;
                    if (it != it_filter->second.end() && it->first > lookAtX) {
                        callback(it->first, it_filter->first, it->second);
                    }
                }
            }
        }
    }
}

note* editTable::clickToAdd(int x, int y) {
    auto p = screenToAbs(x, y);
    automatic(p.X, p.Y);
    auto ptr = addNote(p.X, p.Y, defaultDelay, defaultVolume, defaultInfo);

    printf("mgenner:add %d %d\n", x, y);
    auto hisptr = std::make_shared<history>();  //插入历史记录
    hisptr->method = history::H_NOTE_ADD;
    hisptr->noteIds.push_back(ptr->id);
    pushHistory(hisptr);

    return ptr;
}
void editTable::clearSelected() {
    for (auto it : selected) {
        it->selected = false;
    }
    selected.clear();
    updateSelectedStatus();
}
void editTable::clearNotes() {
    clear();
    selected.clear();
    updateSelectedStatus();
}
void editTable::removeSelected() {
    printf("mgenner:delete notes\n");
    if (selected.empty())
        return;

    auto hisptr = std::make_shared<history>();  //插入历史记录
    hisptr->method = history::H_NOTE_DEL;

    for (auto it : selected) {
        std::unique_ptr<noteInfo> np(new noteInfo(it));
        hisptr->notes.push_back(std::move(np));
        removeNote(it, false);
    }

    selected.clear();

    pushHistory(hisptr);
    updateSelectedStatus();
}
void editTable::resizeSelected(int delta) {
    if (delta == 0) {
        return;
    }
    float shift = delta * maticBlock;  //需要偏移的长度
    //扫描长度是否足够
    for (auto it : selected) {
        if (it->delay + shift <= 10) {
            //长度不足
            return;
        }
    }
    for (auto it : selected) {
        it->delay += shift;
        resizeNote(it);
    }
}

void editTable::renameSelected(const stringPool::stringPtr& n) {
    onUseInfo(n);
    for (auto it : selected) {
        if (!it->info.empty()) {
            if (it->info[0] == '@') {
                removeControl(it->begin, it->info);
            }
        }
        it->info = n;
        if (!n.empty()) {
            if (n[0] == '@') {
                addControl(it->begin, n);
            }
        }
    }
}
int editTable::selectAll() {
    int num;
    if (infoFilter.empty()) {  //无过滤

        for (auto it : notes) {
            it->selected = true;
            selected.insert(it);
            ++num;
        }

    } else {
        for (auto it : notes) {
            if (it->info == infoFilter) {
                it->selected = true;
                selected.insert(it);
                ++num;
            }
        }
    }
    updateSelectedStatus();
    return num;
}
int editTable::clickToSelect(int x, int y) {
    auto p = screenToAbs(x, y);

    auto res = find(
        p, [](note* n, void* arg) {  //调用HBB搜索
            auto self = (editTable*)arg;
            if (!n->selected) {  //未选择就加上选择
                self->selected.insert(n);
                printf("mgenner:select:%f %f %s delay:%f volume:%d\n", n->begin, n->tone, n->info.c_str(), n->delay, n->volume);
                n->selected = true;

                if (n->info != self->defaultInfo) {
                    printf("mgenner:use note name:%s\n", n->info.c_str());
                    self->defaultInfo = n->info;
                }

            } else {
                self->selected.erase(n);  //第二次点击取消选择
                printf("mgenner:unselect:%f %f\n", n->begin, n->tone);
                n->selected = false;
            }
        },
        this);
    updateSelectedStatus();
    return res;
}
int editTable::selectByArea_unique(int x, int y, int len) {
    auto f = HBB::vec(x, 0);
    auto t = HBB::vec(x + len, 128);

    int selNum = 0;

    find(
        f, t, [](note* n, void* arg) {  //调用HBB搜索
            int* num = (int*)arg;
            if (n->selected) {
                ++(*num);
            }
        },
        &selNum);

    if (selNum > 0)
        return 0;

    f.Y = y;
    t.Y = y + 0.9;

    auto res = find(
        f, t, [](note* n, void* arg) {
            auto self = (editTable*)arg;
            if (!n->selected) {  //未选择就加上选择
                self->selected.insert(n);
                n->selected = true;
            }
        },
        this);
    updateSelectedStatus();
    return res;
}
int editTable::selectByArea(int x, int y, int len) {
    auto f = HBB::vec(x, y);
    auto t = HBB::vec(x + len, y + 0.9);

    auto res = find(
        f, t, [](note* n, void* arg) {  //调用HBB搜索
            auto self = (editTable*)arg;
            if (!n->selected) {  //未选择就加上选择
                self->selected.insert(n);
                n->selected = true;
            }
        },
        this);
    updateSelectedStatus();
    return res;
}
int editTable::selectByArea(int selectBoxX, int selectBoxXend, int selectBoxY, int selectBoxYend) {
    int bx, ex, by, ey;
    if (selectBoxX < selectBoxXend) {
        bx = selectBoxX;
        ex = selectBoxXend;
    } else {
        ex = selectBoxX;
        bx = selectBoxXend;
    }
    if (selectBoxY < selectBoxYend) {
        ey = selectBoxY;
        by = selectBoxYend;
    } else {
        by = selectBoxY;
        ey = selectBoxYend;
    }
    auto pb = screenToAbs(bx, by);
    auto pe = screenToAbs(ex, ey);
    auto res = find(
        pb, pe, [](note* n, void* arg) {  //调用HBB搜索
            auto self = (editTable*)arg;
            if (!n->selected) {  //未选择就加上选择
                self->selected.insert(n);
                n->selected = true;
            }
        },
        this);
    updateSelectedStatus();
    return res;
}

void editTable::clickToDisplay(int x, int y) {
    auto p = screenToAbs(x, y);
    automatic(p.X, p.Y);
    if (p.X < 0 || (rawRightMax > 0 && p.X > rawRightMax - 1) || p.Y < 0 || p.Y > pitchNum - 1) {
        return;
    }
    bool res = true;
    struct arg_t {
        bool* res;
        int x;
        int defaultDelay;
    } arg;
    arg.res = &res;
    displayBuffer.clear();
    if (!pasteMode) {
        arg.x = p.X;
        arg.defaultDelay = defaultDelay;
        find(
            HBB::vec(p.X, p.Y + 0.1),
            HBB::vec(p.X + defaultDelay, p.Y + 0.8),
            [](note* n, void* arg) {
                auto self = (arg_t*)arg;
                if (n->begin >= (self->x + self->defaultDelay)) {
                    return;
                }
                if ((n->begin + n->delay) <= self->x) {
                    return;
                }
                *(self->res) = false;
            },
            &arg);
    } else {
        //粘贴模式
        for (auto& it : noteTemplate) {
            arg.defaultDelay = it.dur;
            arg.x = p.X + it.begin;
            find(
                HBB::vec(p.X + it.begin, p.Y + it.tone + 0.1),
                HBB::vec(p.X + it.begin + +it.dur, p.Y + it.tone + 0.8),
                [](note* n, void* arg) {
                    auto self = (arg_t*)arg;
                    if (n->begin >= (self->x + self->defaultDelay)) {
                        return;
                    }
                    if ((n->begin + n->delay) <= self->x) {
                        return;
                    }
                    *(self->res) = false;
                },
                &arg);
        }
    }
    if (!res) {
        return;
    }
    if (!pasteMode) {
        displayBuffer_t tmp;
        tmp.begin = p.X;
        tmp.tone = p.Y;
        tmp.info = defaultInfo;
        tmp.dur = defaultDelay;
        tmp.volume = defaultVolume;
        displayBuffer.push_back(tmp);
    } else {
        for (auto& it : noteTemplate) {
            displayBuffer_t tmp;
            tmp.begin = p.X + it.begin;
            tmp.tone = p.Y + it.tone;
            tmp.info = it.info;
            tmp.dur = it.dur;
            tmp.volume = it.volume;
            displayBuffer.push_back(tmp);
        }
    }
}

void editTable::clickToDisplay_close() {
    displayBuffer.clear();
}

void editTable::addDisplaied() {
    if (!displayBuffer.empty()) {
        auto hisptr = std::make_shared<history>();  //插入历史记录
        hisptr->method = history::H_NOTE_ADD;
        clearSelected();
        for (auto& it : displayBuffer) {
            auto n = addNote(it.begin, it.tone, it.dur, it.volume, it.info);
            n->selected = true;
            selected.insert(n);
            hisptr->noteIds.push_back(n->id);
        }
        updateSelectedStatus();
        pushHistory(hisptr);

        displayBuffer.clear();
    }
    pasteMode = false;
}

void editTable::findNote() {
    noteAreaHeight = ((float)windowHeight) / ((float)noteHeight);  //音符区域高度=窗口高度/音符图片高度
    noteAreaWidth = ((float)windowWidth) / noteLength;             //音符区域宽度=窗口宽度/音符长度比例

    realLookAtY = lookAtY - noteAreaHeight / 2;  //左上角Y位置=中心Y位置-音符区域高度/2
    //realLookAtX=lookAtX

    drawTableRaws();

    drawTableColumns();
    drawSectionLine();

    HBB::vec from;
    from.set(lookAtX, realLookAtY);

    HBB::vec to = from;
    to.X += noteAreaWidth;
    to.Y += noteAreaHeight;

    find(
        from, to, [](note* n, void* arg) {  //调用HBB搜索
            auto self = (editTable*)arg;
            self->drawNoteAbs(n);
        },
        this);
}

void editTable::drawNoteAbs(note* n) {
    drawNoteAbs(n->begin, n->tone, n->delay, n->volume, n->info, n->selected);
}

void editTable::drawTableColumns() {
    float p;
    float r;
    float delta = maticBlock * noteLength;
    if (delta < 5)
        return;
    int befn = lookAtX / maticBlock;
    float start = (befn + 1) * maticBlock;
    r = start;

    //printf("%f %f\n",p,r);

    while (1) {
        p = (r - lookAtX) * noteLength;

        if (p >= windowWidth || (rawRightMax > 0 && r > rawRightMax))
            break;

        if (p > 0 && r > 0)
            drawTimeCol(p);

        r += maticBlock;
    }
}
void editTable::drawTempoLine() {
    drawTempoPadd();
    getTempo(lookAtX, [&](int tick, double tempo) {
        auto p = (tick - lookAtX) * noteLength;
        if (p < 0)
            p = 0;
        if (p > windowWidth) {
            return false;
        } else {
            drawTempo(p, tempo);
        }
        return true;
    });
}
void editTable::drawDescriptionsLine() {
    drawDescriptionsPadd();
    if (infoFilter.empty()) {
        for (auto& it_filter : descriptions) {
            auto it = it_filter.second.upper_bound(lookAtX);  //获取大于tick的第一个元素
            while (it != it_filter.second.end()) {
                auto p = (it->first - lookAtX) * noteLength;
                if (p > windowWidth) {
                    break;
                } else {
                    drawDescriptions(p, it_filter.first, it->second);
                }
                ++it;
            }
        }
    } else {
        auto it_filter = descriptions.find(infoFilter);
        if (it_filter != descriptions.end()) {
            auto it = it_filter->second.upper_bound(lookAtX);  //获取大于tick的第一个元素
            while (it != it_filter->second.end()) {
                auto p = (it->first - lookAtX) * noteLength;
                if (p > windowWidth) {
                    break;
                } else {
                    drawDescriptions(p, it_filter->first, it->second);
                }
                ++it;
            }
        }
    }
}
void editTable::drawSectionLine() {
    float p;
    float r;
    float delta = sectionLen * noteLength;
    if (delta < 5)
        return;
    int befn = lookAtX / sectionLen;
    float start = (befn + 1) * sectionLen;
    r = start;

    while (1) {
        ++befn;

        p = (r - lookAtX) * noteLength;

        if (p >= windowWidth || (rawRightMax > 0 && (befn * sectionLen) > rawRightMax))
            break;

        if (p > 0 && r > 0)
            drawSectionCol(p, befn + 1);

        r += sectionLen;
    }
    if (lookAtX < 0) {
        float posi = (-lookAtX) * noteLength;
        if (posi > 0 && posi < windowWidth)
            drawSectionCol(posi, 1);
    }
}
void editTable::drawTableRaws() {
    int p;
    int ilookAtY = lookAtY;

    rawLeft = (0. - lookAtX) * noteLength;
    if (rawLeft < 0) {
        rawLeft = 0;
    } else if (rawLeft > windowWidth) {
        rawLeft = windowWidth;
    }

    if (rawRightMax <= 0) {
        rawRight = windowWidth;
    } else {
        rawRight = (rawRightMax - lookAtX) * noteLength;
        if (rawRight < 0) {
            rawRight = 0;
        } else if (rawRight > windowWidth) {
            rawRight = windowWidth;
        }
    }

    if (ilookAtY >= 0 && ilookAtY < pitchNum) {
        p = ilookAtY;
        while (1) {
            if (p >= pitchNum || p < 0)
                break;
            if (!drawToneRaw(p))
                break;
            --p;
        }
        p = ilookAtY + 1;
        while (1) {
            if (p >= pitchNum || p < 0)
                break;
            if (!drawToneRaw(p))
                break;
            ++p;
        }
    } else if (ilookAtY < 0) {
        p = 0;
        while (1) {
            if (p >= pitchNum || p < 0)
                break;
            if (!drawToneRaw(p))
                break;
            ++p;
        }
    } else {
        p = pitchNum - 1;
        while (1) {
            if (p >= pitchNum || p < 0)
                break;
            if (!drawToneRaw(p))
                break;
            --p;
        }
    }
}

#define vPosi(n, max) \
    if (n < 0)        \
        n = 0;        \
    else if (n > max) \
        n = max;

bool editTable::drawToneRaw(int t) {
    float relY = t - realLookAtY;
    int scrY = relY * noteHeight;
    int scrYto = windowHeight - scrY;
    scrY = scrYto - noteHeight;

    //vPosi(scrY   ,windowHeight);
    //vPosi(scrYto ,windowHeight);

    //if(scrYto - scrY != noteHeight)
    //    ::__android_log_print(ANDROID_LOG_INFO, "mgenner","%d %d\n",scrY,scrYto);

    if ((scrYto > windowHeight && scrY > windowHeight) || (scrYto < 0 && scrY < 0))
        return false;

    drawTableRaw(scrY, scrYto, rawLeft, rawRight, t);

    return true;
}

void editTable::drawNoteAbs(float begin, float tone, float delay, float volume, const stringPool::stringPtr& info, bool selected, bool onlydisplay) {
    float relX = begin - lookAtX;  //相对坐标
    float relY = tone - realLookAtY;

    int scrX = relX * noteLength;
    int scrY = relY * noteHeight;

    int scrYto = windowHeight - scrY;  //y坐标上下翻转，因为屏幕坐标系和midi坐标系上下相反

    scrY = scrYto - noteHeight;  //翻转过，当然要减

    int scrXto = scrX + delay * noteLength;

    vPosi(scrX, windowWidth);  //验证
    vPosi(scrXto, windowWidth);
    vPosi(scrY, windowHeight);
    vPosi(scrYto, windowHeight);

    if (scrX == scrXto && (scrX == 0 || scrX == windowWidth))
        return;  //长度为0并且在边缘，不予显示（因为显示不出来）

    if (scrY == scrYto && (scrY == 0 || scrY == windowHeight))
        return;  //长度为0并且在边缘，不予显示（因为显示不出来）

    drawNote(scrX, scrY, scrXto, scrYto, volume, info, selected, onlydisplay);
}

void editTable::toString(std::string& str) {
    str = "MGNR V1.0\n";
    char tbuf[1024];
    snprintf(tbuf, sizeof(tbuf), "!T%d\n", TPQ);
    str += tbuf;
    for (auto it : notes) {
        snprintf(tbuf, sizeof(tbuf), "+%s %f %f %f %d\n", it->info.c_str(), it->begin, it->tone, it->delay, it->volume);
        str += tbuf;
    }
    for (auto it : timeMap) {
        snprintf(tbuf, sizeof(tbuf), "!B%d %lf\n", it.first, it.second);
        str += tbuf;
    }
}
void editTable::loadString(const std::string& str) {
    std::istringstream iss(str);
    char buf[1024];
    while (!iss.eof()) {
        bzero(buf, 1024);
        iss.getline(buf, 1024);
        if (buf[0] == '+') {
            if (strlen(buf) > 2) {
                std::istringstream ts(buf + 1);

                std::string info;
                float position;
                float tone;
                float delay;
                int v;

                ts >> info;
                ts >> position;
                ts >> tone;
                ts >> delay;
                ts >> v;

                addNote(position, tone, delay, v, strPool.create(info));
            }
        } else if (buf[0] == '!') {
            if (strlen(buf) >= 3) {
                if (buf[1] == 'T') {
                    int t = atoi(buf + 2);
                    TPQ = t;
                    rebuildNoteLen();
                } else if (buf[1] == 'B') {
                    int tick = 0;
                    double tpo = 120;
                    if (sscanf(buf + 2, "%d %lf", &tick, &tpo) >= 2) {
                        addTempo(tick, tpo);
                    }
                }
            }
        }
    }
}
void editTable::selectedToRelative(std::string& out) {
    std::map<int, std::set<note*> > ns;
    for (auto it : selected) {
        ns[it->begin].insert(it);
    }
    std::list<int> single;
    for (auto it : ns) {
        auto topn = it.second.begin();
        if (topn != it.second.end()) {
            single.push_back((*topn)->tone);
        }
    }
    char buf[64];
    out = "0 ";
    if (!single.empty()) {
        auto it = single.begin();
        int last = *it;
        ++it;
        for (; it != single.end(); ++it) {
            int delta = *it - last;
            last = *it;
            snprintf(buf, 64, "%d ", delta);
            out += buf;
        }
    }
}

void editTable::undo() {
    printf("mgenner:undo");
    editStatus = true;
    auto it = histories_undo.rbegin();
    if (it != histories_undo.rend()) {
        std::shared_ptr<history> step = *it;

        {
            history* h = step.get();
            if (h->method == history::H_NOTE_ADD) {
                h->notes.clear();
                for (auto id : h->noteIds) {
                    auto p = seekNoteById(id);
                    if (p) {
                        std::unique_ptr<noteInfo> np(new noteInfo(p));
                        h->notes.push_back(std::move(np));
                        removeNote(p);
                    }
                }
                h->noteIds.clear();
            } else if (h->method == history::H_NOTE_DEL) {
                h->noteIds.clear();
                for (auto& itn : h->notes) {
                    auto n = addNote(itn->position,
                                     itn->tone,
                                     itn->delay,
                                     itn->volume,
                                     strPool.create(itn->info),
                                     itn->id);
                    //printf("mgenner:id=%d", n->id);
                    h->noteIds.push_back(n->id);
                }
                h->notes.clear();
            } else if (h->method == history::H_TEMPO_ADD) {
                timeMap.erase(h->begin);
            } else if (h->method == history::H_TEMPO_DEL) {
                addTempo(h->begin, h->tempo);
            }
        }

        histories_redo.push_back(step);
        histories_undo.pop_back();
    }
}

void editTable::redo() {
    printf("mgenner:redo");
    editStatus = true;
    auto it = histories_redo.rbegin();
    if (it != histories_redo.rend()) {
        std::shared_ptr<history> step = *it;

        {
            history* h = step.get();
            if (h->method == history::H_NOTE_DEL) {
                h->notes.clear();
                for (auto id : h->noteIds) {
                    auto p = seekNoteById(id);
                    if (p) {
                        std::unique_ptr<noteInfo> np(new noteInfo(p));
                        h->notes.push_back(std::move(np));
                        removeNote(p);
                    }
                }
                h->noteIds.clear();
            } else if (h->method == history::H_NOTE_ADD) {
                h->noteIds.clear();
                for (auto& itn : h->notes) {
                    auto n = addNote(itn->position,
                                     itn->tone,
                                     itn->delay,
                                     itn->volume,
                                     strPool.create(itn->info),
                                     itn->id);
                    //printf("mgenner:id=%d id_set=%d", n->id,itn->id);
                    h->noteIds.push_back(n->id);
                }
                h->notes.clear();
            } else if (h->method == history::H_TEMPO_DEL) {
                timeMap.erase(h->begin);
            } else if (h->method == history::H_TEMPO_ADD) {
                addTempo(h->begin, h->tempo);
            }
        }

        histories_undo.push_back(step);
        histories_redo.pop_back();
    }
}

void editTable::addChord(float position, const std::string& root, const std::string& name, const char* format, float length, int root_base, int v, const std::string& info, bool useTPQ) {
    auto cmnit = chord_map_note.find(root);
    auto cmit = chord_map.find(name);
    if (cmnit == chord_map_note.end() || cmit == chord_map.end() || length <= 0)
        return;

    int lform = strlen(format);

    int root_note = cmnit->second + root_base * 12;
    const std::vector<int>& chord = cmit->second;
    float tm = length / lform;
    float pos = position;
    if (useTPQ) {
        tm *= TPQ;
        pos *= TPQ * length;
    }
    pos += XShift * TPQ;

    auto hisptr = std::make_shared<history>();  //插入历史记录
    hisptr->method = history::H_NOTE_ADD;
    for (int i = 0; i < lform; ++i) {
        int dis = format[i] - '0';
        try {
            if (dis >= 0 && dis < notes.size()) {
                int note = root_note + chord.at(dis) + baseTone;
                hisptr->noteIds.push_back(addNote(pos, note, tm, v, strPool.create(info))->id);
            }
        } catch (...) {
        }
        pos += tm;
    }
    pushHistory(hisptr);
}
void editTable::parseChordNotes(std::vector<int>& notes, const std::string& name) const {
    char buf[128];
    snprintf(buf, 128, "%s", name.c_str());
    char* bufp = buf;
    while (*bufp) {
        if (*bufp == '-')
            *bufp = ' ';
        ++bufp;
    }
    std::string sbuf;
    std::istringstream iss(buf);
    int last = -1;
    while (1) {
        sbuf = "";
        iss >> sbuf;
        auto it = note_number_map.find(sbuf);
        if (it == note_number_map.end()) {
            break;
        }
        int note = it->second;
        while (note < last) {
            note += 12;
        }
        last = note;
        notes.push_back(note);
        //printf("%d\n",note);
    }
}
void editTable::addChord(float position, const std::string& name, float length, int root_base, int v, const std::string& info, const std::string& format, bool useTPQ) {
    std::vector<int> notes;
    parseChordNotes(notes, name);
    float pos = position;
    float len = length;
    if (useTPQ) {
        len *= TPQ;
        pos *= TPQ * length;
    }
    pos += XShift * TPQ;

    auto hisptr = std::make_shared<history>();  //插入历史记录
    hisptr->method = history::H_NOTE_ADD;
    if (format.empty()) {
        for (auto it : notes) {
            int note = root_base * 12 + it + 60 + baseTone;
            hisptr->noteIds.push_back(addNote(pos, note, len, v, strPool.create(info))->id);
        }
    } else {
        auto fstr = format.c_str();
        int flen = strlen(fstr);
        len /= flen;
        for (int i = 0; i < flen; ++i) {
            int dis = fstr[i] - '0';
            try {
                if (dis >= 0 && dis < notes.size()) {
                    int note = root_base * 12 + notes.at(dis) + baseTone;
                    hisptr->noteIds.push_back(addNote(pos, note, len, v, strPool.create(info))->id);
                }
            } catch (...) {
            }
            pos += len;
        }
    }
    pushHistory(hisptr);
}
void editTable::scrollBuilder_process() {
    if (updateTimeMax()) {
        scrollBuilder_it = notes.begin();
        scrollBuilder_processing = true;
        scrollBuilder_onGetNoteArea();
    }
    if (scrollBuilder_processing) {
        int count = 0;
        while (scrollBuilder_it != notes.end()) {
            scrollBuilder_onGetAllNotePos(*scrollBuilder_it);
            ++scrollBuilder_it;
            if ((++count) > 512) {
                break;
            }
        }
        if (scrollBuilder_it == notes.end()) {
            scrollBuilder_processing = false;
            scrollBuilder_onSwap();
        }
    }
}

inline bool isNotHalfNote(int note, int base) {
    const static bool l[] = {true, false, true, false, true, true, false, true, false, true, false, true};
    return l[(note - base + 12) % 12];
}

inline bool checkMajor(int note) {
    const static bool l[] = {true, false, false, false, true, false, false, true, false, false, false, false};
    return l[note % 12];
}

int editTable::getBaseTone() {
    int single_note_count[12];  //记录每个音符出现次数
    for (int i = 0; i < 12; ++i) {
        single_note_count[i] = 0;
    }
    int note_count = 0;
    for (auto note : selected) {
        note_count += note->delay;
        single_note_count[(((int)note->tone) + 12) % 12] += note->delay;
    }
    std::tuple<int, float> major_prob[12];
    for (int base = 0; base < 12; ++base) {
        int nh_count = 0;
        for (int i = 0; i < 12; ++i) {
            if (isNotHalfNote(i, base)) {
                nh_count += single_note_count[i];
            }
        }
        if (note_count == 0) {
            major_prob[base] = std::make_tuple(base, 0);
        } else {
            major_prob[base] = std::make_tuple(base, ((float)nh_count) / ((float)note_count));
        }
    }
    std::sort(major_prob, major_prob + 12,
              [](const std::tuple<int, float>& x, const std::tuple<int, float>& y) {
                  return std::get<1>(x) > std::get<1>(y);
              });
    for (auto it : major_prob) {
        printf("mgenner:%d %f\n", std::get<0>(it),
               std::get<1>(it));
    }
    if (checkMajor(std::get<1>(major_prob[0]))) {
        baseTone = std::get<0>(major_prob[0]);
    } else {
        if (std::get<1>(major_prob[1]) == std::get<1>(major_prob[0])) {
            baseTone = std::get<0>(major_prob[1]);
        } else {
            baseTone = std::get<0>(major_prob[0]);
        }
    }
    //识别大小调
    int num_do = single_note_count[baseTone];
    int num_la = single_note_count[(baseTone + 9) % 12];
    isMajor = (num_do > num_la);
    return baseTone;
}

void editTable::copy() {
    if (selected.empty()) {
        return;
    }
    //计算基点
    float minTime;
    float minTone;
    bool first = true;
    for (auto it : selected) {
        if (first || it->begin < minTime) {
            minTime = it->begin;
        }
        if (first || it->tone < minTone) {
            minTone = it->tone;
        }
        first = false;
    }
    //创建剪贴板
    noteTemplate.clear();
    for (auto it : selected) {
        displayBuffer_t tmp;
        tmp.begin = it->begin - minTime;
        tmp.tone = it->tone - minTone;
        tmp.info = it->info;
        tmp.dur = it->delay;
        tmp.volume = it->volume;
        noteTemplate.push_back(tmp);
    }
}

}  // namespace mgnr

#include "editWindow.h"

editWindow::editWindow(renderContext* p) {
    parent = p;
    snprintf(defaultInfoBuffer, sizeof(defaultInfoBuffer), "%s", defaultInfo.c_str());
    scroll_texture = SDL_CreateTexture(parent->renderer,
                                       SDL_PIXELFORMAT_ABGR8888,
                                       SDL_TEXTUREACCESS_TARGET, 1024, 30);
    scroll_texture_buffer = SDL_CreateTexture(parent->renderer,
                                              SDL_PIXELFORMAT_ABGR8888,
                                              SDL_TEXTUREACCESS_TARGET, 1024, 30);

    setSection(4);
}
editWindow::~editWindow() {
    if (scroll_texture) {
        SDL_DestroyTexture(scroll_texture);
    }
    if (scroll_texture_buffer) {
        SDL_DestroyTexture(scroll_texture_buffer);
    }
}

void editWindow::onSetDefaultInfo(const mgnr::stringPool::stringPtr& info) {
    snprintf(defaultInfoBuffer, sizeof(defaultInfoBuffer), "%s", info.c_str());
}

void editWindow::onUseInfo(const mgnr::stringPool::stringPtr& info) {
    // TODO
}

void editWindow::onSetSection(int sec) {
    // TODO
}

void editWindow::rebuildNoteLen() {
    defaultDelay = noteWidth * TPQ;
    maticBlock = noteWidth * TPQ;
    setSection();
}

void editWindow::onLoadName(const mgnr::stringPool::stringPtr& name) {
    // TODO
}

void editWindow::drawNote_begin() {
}

void editWindow::drawNote(int fx, int fy, int tx, int ty, int volume, const mgnr::stringPool::stringPtr& info, bool selected, bool onlydisplay) {
    SDL_Rect rect;
    rect.x = fx;
    rect.y = fy;
    rect.w = tx - fx;
    rect.h = ty - fy;
    SDL_Rect bd;
    bd.x = fx - 2;
    bd.y = fy - 2;
    bd.w = rect.w + 4;
    bd.h = rect.h + 4;
    if (onlydisplay) {
        SDL_SetRenderDrawColor(parent->renderer, volume, volume, 30, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(parent->renderer, &rect);
    } else if (selected) {
        SDL_SetRenderDrawColor(parent->renderer, 255, 128, 192, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(parent->renderer, &bd);
    } else {
        SDL_SetRenderDrawColor(parent->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(parent->renderer, &bd);
    }

    if (!info.empty()) {
        if (info[0] != '@') {
            unsigned char r, g, b;
            auto it = parent->colors.find(info.c_str());
            if (it == parent->colors.end()) {
                r = rand() % 64;
                g = rand() % 64;
                b = rand() % 64;
                std::array<unsigned char, 3> arr;
                arr[0] = r;
                arr[1] = g;
                arr[2] = b;
                parent->colors[info.c_str()] = arr;
            } else {
                r = it->second[0];
                g = it->second[1];
                b = it->second[2];
            }
            SDL_SetRenderDrawColor(parent->renderer, r + volume, g + volume, b + volume, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(parent->renderer, &rect);
        } else {
            SDL_SetRenderDrawColor(parent->renderer, 128, 128, 192, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(parent->renderer, &rect);
        }
    } else {
        SDL_SetRenderDrawColor(parent->renderer, 64 + volume, 64 + volume, 30, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(parent->renderer, &rect);
    }

    if (noteHeight >= 20 && !info.empty() && infoFilter.empty() && (selected || info[0] == '@')) {
        SDL_Color textColor = {255, 255, 255};
        SDL_Texture* tex = parent->getText(info.value(), textColor, rect);
        SDL_RenderCopy(parent->renderer, tex, NULL, &rect);
    }
}

void editWindow::drawNote_end() {
    if (parent->selectingByBox) {
        int bx, ex, by, ey;
        if (parent->selectBoxX < parent->selectBoxXend) {
            bx = parent->selectBoxX;
            ex = parent->selectBoxXend;
        } else {
            ex = parent->selectBoxX;
            bx = parent->selectBoxXend;
        }
        if (parent->selectBoxY < parent->selectBoxYend) {
            by = parent->selectBoxY;
            ey = parent->selectBoxYend;
        } else {
            ey = parent->selectBoxY;
            by = parent->selectBoxYend;
        }
        if (!(bx == ex || by == ey)) {
            SDL_Rect rect;
            rect.x = bx;
            rect.y = by;
            rect.w = 1;
            rect.h = ey - by;
            SDL_SetRenderDrawColor(parent->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(parent->renderer, &rect);
            rect.x = ex;
            SDL_RenderFillRect(parent->renderer, &rect);

            rect.x = bx;
            rect.y = by;
            rect.h = 1;
            rect.w = ex - bx;
            SDL_RenderFillRect(parent->renderer, &rect);
            rect.y = ey;
            SDL_RenderFillRect(parent->renderer, &rect);
            /*
            rect.x=bx;
            rect.y=by;
            rect.h=1;
            rect.w=ey-by;
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255 , 255 , 255));
            rect.x=ex;
            SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 255 , 255 , 255));
             */
        }
    }
}

void editWindow::drawTableRaw(int from, int to, int left, int right, int t) {
    // TODO
    SDL_Rect rect;
    rect.x = 0;
    rect.y = from;
    rect.w = windowWidth;
    rect.h = to - from;

    int k = t % 12;

    static const int pianoKey[] = {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1};

    static const int pianoColorR[] = {20, 10, 20, 10, 20, 10, 10, 20, 10, 20, 10, 10};
    static const int pianoColorG[] = {20, 10, 20, 10, 20, 20, 10, 20, 10, 20, 10, 20};
    static const int pianoColorB[] = {30, 20, 30, 20, 30, 30, 20, 30, 20, 30, 20, 30};

    SDL_SetRenderDrawColor(parent->renderer, pianoColorR[k], pianoColorG[k], pianoColorB[k], SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(parent->renderer, &rect);

    if (t >= 0 && t < 128) {
        rect.w = 30;
        if (pianoKey[k] == 1)
            SDL_SetRenderDrawColor(parent->renderer, 190, 190, 170, SDL_ALPHA_OPAQUE);
        else
            SDL_SetRenderDrawColor(parent->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

        SDL_RenderFillRect(parent->renderer, &rect);
        if (noteHeight >= 20) {
            rect.w = std::get<1>(parent->toneMap[t]);
            rect.h = std::get<2>(parent->toneMap[t]);
            SDL_RenderCopy(parent->renderer, std::get<0>(parent->toneMap[t]), NULL, &rect);
        }
    }
}

void editWindow::drawTimeCol(float p) {
    SDL_Rect rect;
    rect.x = p;
    rect.y = 0;
    rect.w = 2;
    rect.h = windowHeight;
    SDL_SetRenderDrawColor(parent->renderer, 5, 5, 20, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(parent->renderer, &rect);
}

void editWindow::drawSectionCol(float p, int n) {
    SDL_Rect rect;
    rect.x = p;
    rect.y = 0;
    rect.w = 3;
    rect.h = windowHeight;
    SDL_SetRenderDrawColor(parent->renderer, 5, 5, 5, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(parent->renderer, &rect);

    rect.y = windowHeight - 90;
    rect.h = 30;
    SDL_Color textColor = {64, 128, 128};
    char buf[64];
    snprintf(buf, 64, "%d", n);
    auto tex = parent->getText(buf, textColor, rect);
    SDL_RenderCopy(parent->renderer, tex, NULL, &rect);
}

void editWindow::drawTempo(float p, double t) {
    SDL_Rect rect;
    rect.x = p;
    rect.y = 0;
    rect.w = 1;
    rect.h = windowHeight - 30;
    SDL_SetRenderDrawColor(parent->renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(parent->renderer, &rect);

    rect.y = windowHeight - 60;
    rect.h = 30;
    SDL_Color textColor = {64, 128, 128};
    char buf[64];
    snprintf(buf, 64, "BPM=%d", (int)round(t));
    auto tex = parent->getText(buf, textColor, rect);
    SDL_RenderCopy(parent->renderer, tex, NULL, &rect);
}

void editWindow::drawDescriptions(float p, const mgnr::stringPool::stringPtr& title, const std::string& content) {
    SDL_Rect rect;
    rect.x = p;
    rect.y = 0;
    rect.w = 1;
    rect.h = windowHeight - 60;
    SDL_Texture* tex;
    SDL_Color textColor = {64, 128, 128};
    //分割线
    SDL_SetRenderDrawColor(parent->renderer, 64, 128, 128, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(parent->renderer, &rect);
    rect.y = parent->menuHeight;
    //标题
    tex = parent->getText(title.value(), textColor, rect);
    SDL_RenderCopy(parent->renderer, tex, NULL, &rect);
    //内容
    rect.y = parent->menuHeight + 20;
    tex = parent->getText(content, textColor, rect);
    SDL_RenderCopy(parent->renderer, tex, NULL, &rect);
}

void editWindow::drawDescriptionsPadd() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = windowWidth;
    rect.h = 40 + parent->menuHeight;
    SDL_SetRenderDrawColor(parent->renderer, 0, 0, 30, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(parent->renderer, &rect);
}

void editWindow::drawTempoPadd() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = windowHeight - 60;
    rect.w = windowWidth;
    rect.h = 30;
    SDL_SetRenderDrawColor(parent->renderer, 0, 0, 30, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(parent->renderer, &rect);
}

void editWindow::drawScroll() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = windowHeight - 30;
    rect.w = windowWidth;
    rect.h = 30;
    SDL_SetRenderDrawColor(parent->renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(parent->renderer, &rect);
    if (scroll_texture) {
        SDL_RenderCopy(parent->renderer, scroll_texture, NULL, &rect);
    }
    int m = noteTimeMax;
    if (m > 0) {
        rect.x = (lookAtX * windowWidth) / m;
        rect.w = 1;
        SDL_SetRenderDrawColor(parent->renderer, 128, 255, 128, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(parent->renderer, &rect);
    }
}

void editWindow::onSelectedChange(int len) {
    // TODO
}
void editWindow::onScriptCmd(const char* cmd) {
    // TODO
}

void editWindow::drawCaption(float p, const std::string& s) {
    SDL_Rect rect;
    rect.x = p;
    rect.y = windowHeight - 150;
    SDL_Color textColor = {64, 64, 5};
    SDL_Texture* tex = parent->getText(s, textColor, rect);
    SDL_RenderCopy(parent->renderer, tex, NULL, &rect);
}

void editWindow::draw() {
    this->render();
    if (lastDefaultInfo != defaultInfo) {
        lastDefaultInfo = defaultInfo;
        onSetDefaultInfo(defaultInfo);
    }
    if (section != lastSection) {
        lastSection = section;
        onSetSection(section);
    }
    if (needUpdateWindowTitle) {
        needUpdateWindowTitle = false;
        updateWindowTitle_process();
    }
}

void editWindow::playStep() {
    this->mgnr::player::playStep();
}

void editWindow::scrollBuilder_onGetNoteArea() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 1024;
    rect.h = 30;
    SDL_RenderClear(parent->renderer);
    SDL_SetRenderDrawColor(parent->renderer, 0, 0, 20, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(parent->renderer, &rect);
    hlen = noteToneMax - noteToneMin;  //纵向的距离
    nmax = noteToneMax;
    nmin = noteToneMin;
    if (hlen < 30) {
        int s = (noteToneMax + noteToneMin) / 2;
        hlen = 30;
        nmax = s + 15;
        nmin = s - 15;
    }
}
void editWindow::scrollBuilder_onGetAllNotePos(mgnr::note* n) {
    SDL_Rect rect;
    rect.h = 1;
    if (n->tone > nmin && n->tone < nmax) {
        rect.y = 30 - ((n->tone - nmin) * 30) / hlen;
        rect.w = (n->delay * 1024) / noteTimeMax;
        if (rect.w <= 0)
            rect.w = 1;
        rect.x = (n->begin * 1024) / noteTimeMax;

        SDL_SetRenderDrawColor(parent->renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(parent->renderer, &rect);
    }
}
void editWindow::scrollBuilder_onSwap() {
    auto tmp = scroll_texture;
    scroll_texture = scroll_texture_buffer;
    scroll_texture_buffer = tmp;
}
void editWindow::buildScroll() {
    SDL_SetRenderTarget(parent->renderer, scroll_texture_buffer);
    scrollBuilder_process();
    SDL_SetRenderTarget(parent->renderer, nullptr);
}
void editWindow::hideMode() {
    if (infoFilter.empty()) {
        infoFilter = defaultInfo;
    } else {
        infoFilter.clear();
    }
}

void editWindow::setInfo(const std::string& str) {
    if (str.empty()) {
        return;
    }
    this->defaultInfo = this->strPool.create(str);
    if (!this->infoFilter.empty()) {
        this->infoFilter = this->defaultInfo;
    }
    if (show_edit_window) {
        for (auto& it : this->selected) {
            it->info = this->strPool.create(str);
        }
    }
}

void editWindow::editStatusUpdate() {
    needUpdateWindowTitle = true;
}

void editWindow::updateWindowTitle_process() {
    std::string title = "";
    if (editStatus) {
        title += "(*)";
    }
    if (!midiFilePath.empty()) {
        title += midiFilePath;
    }
    title += " mGenNer";
    SDL_SetWindowTitle(parent->window, title.c_str());
}

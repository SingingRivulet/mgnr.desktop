#include "mgenner.h"
mgenner::mgenner()
    : fileDialog_saveMidi(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir) {
    printf("mgenner:init...\n");
    loadConfig();
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    window = SDL_CreateWindow("mgnr",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              1200,
                              600,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
    windowWidth = 1200;
    windowHeight = 600;
    snprintf(defaultInfoBuffer, sizeof(defaultInfoBuffer), "%s", defaultInfo.c_str());
    TTF_Init();
    font = TTF_OpenFont(path_font.c_str(), 20);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    ui_init();
    synth_init();

    setSection(4);
    toneMapInit();

    scroll_texture = SDL_CreateTexture(renderer,
                                       SDL_PIXELFORMAT_ABGR8888,
                                       SDL_TEXTUREACCESS_TARGET, 1024, 30);
    scroll_texture_buffer = SDL_CreateTexture(renderer,
                                              SDL_PIXELFORMAT_ABGR8888,
                                              SDL_TEXTUREACCESS_TARGET, 1024, 30);
    updateWindowTitle();
    printf("mgenner:start\n");
}
mgenner::~mgenner() {
    printf("mgenner:release mgenner\n");
    if (scroll_texture) {
        SDL_DestroyTexture(scroll_texture);
    }
    if (scroll_texture_buffer) {
        SDL_DestroyTexture(scroll_texture_buffer);
    }
    shutdownPlugins();
    ui_shutdown();
    synth_shutdown();
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}

void mgenner::toneMapInit() {
    SDL_Color textColor = {255, 255, 255};
    static const char* pianoKeyName[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    for (int i = 0; i < 128; i++) {
        int k = i % 12;
        char buf[32];
        if (k == 0)
            snprintf(buf, 32, "%s\t%d", pianoKeyName[k], (i / 12));
        else
            snprintf(buf, 32, "%s", pianoKeyName[k]);
        auto s = TTF_RenderUTF8_Solid(font, buf, textColor);
        toneMap[i] = std::tuple<SDL_Texture*, int, int>(
            SDL_CreateTextureFromSurface(renderer, s),
            s->w, s->h);
        SDL_FreeSurface(s);
    }
}

void mgenner::loop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_WINDOWEVENT &&
                   event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            windowWidth = event.window.data1;
            windowHeight = event.window.data2;
        } else if (event.type == SDL_KEYDOWN &&
                   (event.key.keysym.sym == SDLK_LCTRL ||
                    event.key.keysym.sym == SDLK_RCTRL)) {
            button_ctrl = true;
        } else if (event.type == SDL_KEYUP &&
                   (event.key.keysym.sym == SDLK_LCTRL ||
                    event.key.keysym.sym == SDLK_RCTRL)) {
            button_ctrl = false;
        } else if (event.type == SDL_KEYDOWN &&
                   (event.key.keysym.sym == SDLK_LSHIFT ||
                    event.key.keysym.sym == SDLK_RSHIFT)) {
            button_shift = true;
        } else if (event.type == SDL_KEYUP &&
                   (event.key.keysym.sym == SDLK_LSHIFT ||
                    event.key.keysym.sym == SDLK_RSHIFT)) {
            button_shift = false;
        } else {
            events.push_back(event);
        }
    }
    focusCanvas = true;
    playStep();
    draw();
    if (focusCanvas) {
        processEvents();
    }
    events.clear();
}

void mgenner::onSetDefaultInfo(const mgnr::stringPool::stringPtr& info) {
    snprintf(defaultInfoBuffer, sizeof(defaultInfoBuffer), "%s", info.c_str());
}

void mgenner::onUseInfo(const mgnr::stringPool::stringPtr& info) {
    // TODO
}

void mgenner::onSetSection(int sec) {
    // TODO
}

void mgenner::rebuildNoteLen() {
    defaultDelay = noteWidth * TPQ;
    maticBlock = noteWidth * TPQ;
    setSection();
}

void mgenner::onLoadName(const mgnr::stringPool::stringPtr& name) {
    // TODO
}

void mgenner::drawNote_begin() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = windowWidth;
    rect.h = windowHeight;
    SDL_SetRenderDrawColor(renderer, 0, 0, 30, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);
}

void mgenner::drawNote(int fx, int fy, int tx, int ty, int volume, const mgnr::stringPool::stringPtr& info, bool selected, bool onlydisplay) {
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
        SDL_SetRenderDrawColor(renderer, volume, volume, 30, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &rect);
    } else if (selected) {
        SDL_SetRenderDrawColor(renderer, 255, 128, 192, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &bd);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &bd);
    }

    if (!info.empty()) {
        if (info[0] != '@') {
            unsigned char r, g, b;
            auto it = colors.find(info.c_str());
            if (it == colors.end()) {
                r = rand() % 64;
                g = rand() % 64;
                b = rand() % 64;
                std::array<unsigned char, 3> arr;
                arr[0] = r;
                arr[1] = g;
                arr[2] = b;
                colors[info.c_str()] = arr;
            } else {
                r = it->second[0];
                g = it->second[1];
                b = it->second[2];
            }
            SDL_SetRenderDrawColor(renderer, r + volume, g + volume, b + volume, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(renderer, &rect);
        } else {
            SDL_SetRenderDrawColor(renderer, 128, 128, 192, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(renderer, &rect);
        }
    } else {
        SDL_SetRenderDrawColor(renderer, 64 + volume, 64 + volume, 30, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &rect);
    }

    if (noteHeight >= 20 && !info.empty() && infoFilter.empty() && (selected || info[0] == '@')) {
        SDL_Color textColor = {255, 255, 255};
        SDL_Texture* tex = getText(info.value(), textColor, rect);
        SDL_RenderCopy(renderer, tex, NULL, &rect);
    }
}

void mgenner::drawNote_end() {
    if (selectingByBox) {
        int bx, ex, by, ey;
        if (selectBoxX < selectBoxXend) {
            bx = selectBoxX;
            ex = selectBoxXend;
        } else {
            ex = selectBoxX;
            bx = selectBoxXend;
        }
        if (selectBoxY < selectBoxYend) {
            by = selectBoxY;
            ey = selectBoxYend;
        } else {
            ey = selectBoxY;
            by = selectBoxYend;
        }
        if (!(bx == ex || by == ey)) {
            SDL_Rect rect;
            rect.x = bx;
            rect.y = by;
            rect.w = 1;
            rect.h = ey - by;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(renderer, &rect);
            rect.x = ex;
            SDL_RenderFillRect(renderer, &rect);

            rect.x = bx;
            rect.y = by;
            rect.h = 1;
            rect.w = ex - bx;
            SDL_RenderFillRect(renderer, &rect);
            rect.y = ey;
            SDL_RenderFillRect(renderer, &rect);
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

void mgenner::drawTableRaw(int from, int to, int left, int right, int t) {
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

    SDL_SetRenderDrawColor(renderer, pianoColorR[k], pianoColorG[k], pianoColorB[k], SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);

    if (t >= 0 && t < 128) {
        rect.w = 30;
        if (pianoKey[k] == 1)
            SDL_SetRenderDrawColor(renderer, 190, 190, 170, SDL_ALPHA_OPAQUE);
        else
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

        SDL_RenderFillRect(renderer, &rect);
        if (noteHeight >= 20) {
            rect.w = std::get<1>(toneMap[t]);
            rect.h = std::get<2>(toneMap[t]);
            SDL_RenderCopy(renderer, std::get<0>(toneMap[t]), NULL, &rect);
        }
    }
}

void mgenner::drawTimeCol(float p) {
    SDL_Rect rect;
    rect.x = p;
    rect.y = 0;
    rect.w = 2;
    rect.h = windowHeight;
    SDL_SetRenderDrawColor(renderer, 5, 5, 20, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);
}

void mgenner::drawSectionCol(float p, int n) {
    SDL_Rect rect;
    rect.x = p;
    rect.y = 0;
    rect.w = 3;
    rect.h = windowHeight;
    SDL_SetRenderDrawColor(renderer, 5, 5, 5, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);

    rect.y = windowHeight - 90;
    rect.h = 30;
    SDL_Color textColor = {64, 128, 128};
    char buf[64];
    snprintf(buf, 64, "%d", n);
    auto tex = getText(buf, textColor, rect);
    SDL_RenderCopy(renderer, tex, NULL, &rect);
}

void mgenner::drawTempo(float p, double t) {
    SDL_Rect rect;
    rect.x = p;
    rect.y = 0;
    rect.w = 1;
    rect.h = windowHeight - 30;
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);

    rect.y = windowHeight - 60;
    rect.h = 30;
    SDL_Color textColor = {64, 128, 128};
    char buf[64];
    snprintf(buf, 64, "BPM=%d", (int)round(t));
    auto tex = getText(buf, textColor, rect);
    SDL_RenderCopy(renderer, tex, NULL, &rect);
}

void mgenner::drawDescriptions(float p, const mgnr::stringPool::stringPtr& title, const std::string& content) {
    SDL_Rect rect;
    rect.x = p;
    rect.y = 0;
    rect.w = 1;
    rect.h = windowHeight - 90;
    SDL_Texture* tex;
    SDL_Color textColor = {64, 128, 128};
    //分割线
    SDL_SetRenderDrawColor(renderer, 64, 128, 128, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);
    //标题
    tex = getText(title.value(), textColor, rect);
    SDL_RenderCopy(renderer, tex, NULL, &rect);
    //内容
    rect.y = 20;
    tex = getText(content, textColor, rect);
    SDL_RenderCopy(renderer, tex, NULL, &rect);
}

void mgenner::drawDescriptionsPadd() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = windowWidth;
    rect.h = 40;
    SDL_SetRenderDrawColor(renderer, 0, 0, 30, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);
}

void mgenner::drawTempoPadd() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = windowHeight - 60;
    rect.w = windowWidth;
    rect.h = 30;
    SDL_SetRenderDrawColor(renderer, 0, 0, 30, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);
}

void mgenner::drawScroll() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = windowHeight - 30;
    rect.w = windowWidth;
    rect.h = 30;
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);
    if (scroll_texture) {
        SDL_RenderCopy(renderer, scroll_texture, NULL, &rect);
    }
    int m = noteTimeMax;
    if (m > 0) {
        rect.x = (lookAtX * windowWidth) / m;
        rect.w = 1;
        SDL_SetRenderDrawColor(renderer, 128, 255, 128, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &rect);
    }
}

void mgenner::onSelectedChange(int len) {
    // TODO
}
void mgenner::onScriptCmd(const char* cmd) {
    // TODO
}

void mgenner::drawCaption(float p, const std::string& s) {
    SDL_Rect rect;
    rect.x = p;
    rect.y = windowHeight - 150;
    SDL_Color textColor = {64, 64, 5};
    SDL_Texture* tex = getText(s, textColor, rect);
    SDL_RenderCopy(renderer, tex, NULL, &rect);
}

void mgenner::draw() {
    this->buildScroll();
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ui_loop();

    SDL_RenderClear(renderer);
    this->render();
    if (lastDefaultInfo != defaultInfo) {
        lastDefaultInfo = defaultInfo;
        onSetDefaultInfo(defaultInfo);
    }
    if (section != lastSection) {
        lastSection = section;
        onSetSection(section);
    }
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
}

void mgenner::playStep() {
    this->mgnr::player::playStep();
}

void mgenner::scrollBuilder_onGetNoteArea() {
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 1024;
    rect.h = 30;
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 20, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);
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
void mgenner::scrollBuilder_onGetAllNotePos(mgnr::note* n) {
    SDL_Rect rect;
    rect.h = 1;
    if (n->tone > nmin && n->tone < nmax) {
        rect.y = 30 - ((n->tone - nmin) * 30) / hlen;
        rect.w = (n->delay * 1024) / noteTimeMax;
        if (rect.w <= 0)
            rect.w = 1;
        rect.x = (n->begin * 1024) / noteTimeMax;

        SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &rect);
    }
}
void mgenner::scrollBuilder_onSwap() {
    auto tmp = scroll_texture;
    scroll_texture = scroll_texture_buffer;
    scroll_texture_buffer = tmp;
}
void mgenner::buildScroll() {
    SDL_SetRenderTarget(renderer, scroll_texture_buffer);
    scrollBuilder_process();
    SDL_SetRenderTarget(renderer, nullptr);
}
void mgenner::hideMode() {
    if (infoFilter.empty()) {
        infoFilter = defaultInfo;
    } else {
        infoFilter.clear();
    }
}

SDL_Texture* mgenner::getText(const std::string& str,
                              const SDL_Color& textColor,
                              SDL_Rect& rect) {
    SDL_Texture* tex;
    std::tuple<int, int, int> color(textColor.r, textColor.g, textColor.b);
    auto it = words.find(str);
    if (it != words.end()) {
        auto itc = it->second.find(color);
        if (itc != it->second.end()) {
            tex = std::get<0>(itc->second);
            rect.w = std::get<1>(itc->second);
            rect.h = std::get<2>(itc->second);
            return tex;
        }
    }
    auto msg = TTF_RenderUTF8_Solid(font, str.c_str(), textColor);
    rect.w = msg->w;
    rect.h = msg->h;
    tex = SDL_CreateTextureFromSurface(renderer, msg);
    words[str][color] =
        std::tuple<SDL_Texture*, int, int>(
            tex,
            msg->w, msg->h);
    SDL_FreeSurface(msg);
    return tex;
}
void mgenner::setInfo(const std::string& str) {
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

void mgenner::editStatusUpdate() {
    updateWindowTitle();
}

void mgenner::updateWindowTitle() {
    std::string title = "";
    if (editStatus) {
        title += "(*)";
    }
    if (!midiFilePath.empty()) {
        title += midiFilePath;
    }
    title += " mGenNer";
    SDL_SetWindowTitle(window, title.c_str());
}

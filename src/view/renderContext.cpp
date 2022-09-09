#include "editWindow.h"
renderContext::renderContext()
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
    TTF_Init();
    font = TTF_OpenFont(path_font.c_str(), 20);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    toneMapInit();
    ui_init();
    synth_init();
}
renderContext::~renderContext() {
    printf("mgenner:release mgenner\n");

    for (int i = 0; i < 128; i++) {
        SDL_DestroyTexture(std::get<0>(toneMap[i]));
    }
    for (auto& it1 : words) {
        for (auto& it2 : it1.second) {
            SDL_DestroyTexture(std::get<0>(it2.second));
        }
    }
    words.clear();
    shutdownModules();
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
void renderContext::toneMapInit() {
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

void renderContext::loop() {
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
        } else if (event.type == SDL_MOUSEMOTION) {
            mouse_x = event.motion.x;
            mouse_y = event.motion.y;
            events.push_back(event);
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

void renderContext::draw() {
    if (drawing) {
        drawing->windowHeight = windowHeight;
        drawing->windowWidth = windowWidth;
    }
    for (auto& it : editWindows) {
        it.second->buildScroll();
    }
    this->module_loop();
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ui_loop();

    SDL_RenderClear(renderer);

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = windowWidth;
    rect.h = windowHeight;
    SDL_SetRenderDrawColor(renderer, 0, 0, 30, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(renderer, &rect);

    if (drawing) {
        drawing->draw();
    }
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
}

void renderContext::playStep() {
    for (auto& it : editWindows) {
        it.second->playStep();
    }
}

SDL_Texture* renderContext::getText(const std::string& str,
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

std::tuple<int, editWindow*> renderContext::createWindow() {
    std::unique_ptr<editWindow> p(new editWindow(this));
    if (drawing == nullptr) {
        showWindow(p.get());
    }
    int id = ++windows_current_id;
    auto ptr = p.get();
    editWindows[id] = std::move(p);
    return std::make_tuple(id, ptr);
}
void renderContext::closeWindow(int id) {
    auto it = editWindows.find(id);
    if (it != editWindows.end()) {
        if (drawing == it->second.get()) {
            drawing = nullptr;
        }
        editWindows.erase(it);
    }
    if (drawing == nullptr) {
        auto it = editWindows.begin();
        if (it != editWindows.end()) {
            showWindow(it->second.get());
        }
    }
}
void renderContext::showWindow(editWindow* w) {
    drawing = w;
    drawing->needUpdateWindowTitle = true;
}
void renderContext::showWindow(int id) {
    auto it = editWindows.find(id);
    if (it != editWindows.end()) {
        showWindow(it->second.get());
    }
}
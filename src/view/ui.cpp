#include "mgenner.h"

void mgenner::ui_init() {
    //创建imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->AddFontFromFileTTF(path_font.c_str(),
                                 20.f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);
    fileDialog_loadMidi.SetTitle("选择文件");
    fileDialog_saveMidi.SetTitle("保存文件");
}

void mgenner::ui_shutdown() {
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void mgenner::ui_loop() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("播放控制", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar)) {
        checkfocus();

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("文件")) {
                if (ImGui::MenuItem("载入", "ctrl+o")) {
                    loadMidiDialog();
                }
                if (ImGui::MenuItem("保存", "ctrl+s")) {
                    saveMidiFile();
                }
                if (ImGui::MenuItem("另存为", "ctrl+shift+s")) {
                    saveMidiDialog();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("设置")) {
                if (ImGui::MenuItem("音轨映射表")) {
                    show_trackMap_window = true;
                    trackMapBuffer_closeWithSave = false;
                    std::map<std::string, int> tn = trackNameMapper;
                    std::map<int, int> ti = trackInsMapper;
                    checkTrackMapper(tn, ti);
                    trackMapBuffer_init(tn, ti);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip(
                        "保存时若音轨映射关系错误，\n"
                        "会自动呼出此菜单");
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("扩展")) {
                plugin_menu();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        if (ImGui::Button("开头")) {
            lookAtX = 0;
        }
        ImGui::SameLine();
        if (!playingStatus) {
            if (ImGui::Button("播放")) {
                playStart();
            }
        } else {
            if (ImGui::Button("暂停")) {
                playStop();
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("空格键");
        }
        ImGui::SameLine();
        if (ImGui::Button("结尾")) {
            lookAtX = noteTimeMax;
        }

        if (fileDialog_loadMidi.HasSelected()) {
            loadMidiFile(fileDialog_loadMidi.GetSelected().string());
            fileDialog_loadMidi.ClearSelected();
        }
        if (fileDialog_saveMidi.HasSelected()) {
            printf("mgenner:save:%s\n", fileDialog_saveMidi.GetSelected().string().c_str());
            saveMidiFile(fileDialog_saveMidi.GetSelected().string());
            fileDialog_saveMidi.ClearSelected();
        }

        if (ImGui::Checkbox("编辑模式", &show_edit_window)) {
            if (show_edit_window) {
                show_trackSelect_window = true;
            }
        }

        if (ImGui::Checkbox("聚焦音轨", &focus_note)) {
            clearSelected();
            if (focus_note) {
                infoFilter = defaultInfo;
                show_trackSelect_window = true;
            } else {
                infoFilter.clear();
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("隐藏其他音轨，仅显示此音轨");
        }

        ImGui::End();
    }
    if (show_edit_window) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth - 420, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("编辑", &show_edit_window, ImGuiWindowFlags_AlwaysAutoResize);
        checkfocus();

        {
            if (ImGui::Button("撤销")) {
                undo();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("ctrl+z");
            }
            ImGui::SameLine();
            if (ImGui::Button("重做")) {
                redo();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("ctrl+shift+z");
            }
            ImGui::SameLine();
            if (ImGui::Button("粘贴")) {
                pasteMode = true;
                selectByBox = false;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("ctrl+v");
            }

            if (ImGui::Button("删除音符")) {
                removeSelected();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("back/delete");
            }
            ImGui::SameLine();
            if (selectByBox) {
                if (ImGui::Button("取消框选")) {
                    selectByBox = false;
                }
            } else {
                if (ImGui::Button("框选模式")) {
                    selectByBox = true;
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("s键");
            }
            ImGui::SameLine();
            if (ImGui::Button("清除选择")) {
                clearSelected();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("鼠标右键");
            }
        }

        if (ImGui::InputInt("TPQ", &TPQ)) {
            if (TPQ > 4096) {
                TPQ = 4096;
            }
            if (TPQ < 1) {
                TPQ = 1;
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("TPQ，即每四分音符的tick数");
        }

        if (ImGui::InputText("音轨命名", defaultInfoBuffer, sizeof(defaultInfoBuffer))) {
            if (strlen(defaultInfoBuffer) <= 1) {
            } else {
                setInfo(defaultInfoBuffer);
                show_trackSelect_window = true;
            }
        }
        if (ImGui::IsItemHovered()) {
            if (!selected.empty()) {
                ImGui::SetTooltip(
                    "改变音轨命名将同时改变所有已选中的音符\n"
                    "\n"
                    "音轨名与所使用的乐器相关联，当编辑对象\n"
                    "为midi时(默认为此格式)，格式为\n"
                    "“乐器名.轨道号”，例如“Piano.0”\n"
                    "编辑其他类型数据时，请参考对应的文档。");
            } else {
                ImGui::SetTooltip(
                    "音轨名与所使用的乐器相关联，当编辑对象\n"
                    "为midi时(默认为此格式)，格式为\n"
                    "“乐器名.轨道号”，例如“Piano.0”\n"
                    "编辑其他类型数据时，请参考对应的文档。");
            }
        }

        if (ImGui::SliderInt("响度", &defaultVolume, 1, 127)) {
            for (auto& it : this->selected) {
                it->volume = defaultVolume;
            }
        }
        if (ImGui::IsItemHovered() && !selected.empty()) {
            ImGui::SetTooltip("改变响度将同时改变所有已选中的音符");
        }

        int sec = section;
        if (ImGui::InputInt("节奏", &sec)) {
            setSection(sec);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("以四分音符为一拍，每小节%d拍", section);
        }

        const static char* noteWidth_items[] = {"1/32", "1/16", "1/8 ", "1/4 ", "1/2 ", "4/3 ", " 1  "};
        const static float noteWidth_items_lens[] = {1.0 / 8.0, 1.0 / 4.0, 1 / 2.0, 1.0, 2.0, 3.0, 4.0};
        ImGui::Text("音符长度");
        ImGui::SameLine();
        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
            noteWidth_items_id--;
            if (noteWidth_items_id < 0) {
                noteWidth_items_id = 0;
            }
            noteWidth = noteWidth_items_lens[noteWidth_items_id];
            rebuildNoteLen();
        }
        ImGui::SameLine(0.0f, spacing);
        ImGui::Text("%s", noteWidth_items[noteWidth_items_id]);
        ImGui::SameLine(0.0f, spacing);
        if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
            noteWidth_items_id++;
            if (noteWidth_items_id > 6) {
                noteWidth_items_id = 6;
            }
            noteWidth = noteWidth_items_lens[noteWidth_items_id];
            rebuildNoteLen();
        }

        ImGui::End();
    }
    if (show_trackSelect_window) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth - 300, 260), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, windowHeight - 260), ImGuiCond_FirstUseEver);
        ImGui::Begin("选择音轨", &show_trackSelect_window);
        checkfocus();

        constexpr char editTractContent[] =
            "编辑状态下更换音轨会同时将所有已选\n"
            "的音符设置为此音轨。\n"
            "如无须此操作，请先取消选中所有音符\n"
            "（在midi界面上单击鼠标右键）";

        if (ImGui::CollapsingHeader("用户音轨", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto& it : strPool.indexer) {
                if (ImGui::Selectable(it.first.c_str(), it.first == defaultInfo.value())) {
                    setInfo(it.first);
                }
                if (ImGui::IsItemHovered() && show_edit_window && !selected.empty()) {
                    ImGui::SetTooltip(editTractContent);
                }
            }
        }
        if (ImGui::CollapsingHeader("系统乐器")) {
            for (int i = 0; i < 128; ++i) {
                if (ImGui::Selectable(mgnr::instrumentName[i], false)) {
                    setInfo(mgnr::instrumentName[i]);
                }
                if (ImGui::IsItemHovered() && show_edit_window && !selected.empty()) {
                    ImGui::SetTooltip(editTractContent);
                }
            }
        }

        ImGui::End();
    }
    if (show_tempoSet_bar) {
        ImGui::SetNextWindowPos(
            ImVec2(show_tempoSet_bar_pos_x,
                   show_tempoSet_bar_pos_y - 40));

        ImGui::Begin("设置速度", &show_tempoSet_bar,
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoBringToFrontOnFocus);
        checkfocus();
        if (!(ImGui::IsItemFocused() || ImGui::IsWindowFocused())) {
            show_tempoSet_bar = false;
        }
        if (ImGui::Selectable("设置速度")) {
            show_tempoSet_bar = false;
            show_tempoAdd_bar = true;
            auto p = screenToAbs(show_tempoSet_bar_pos_x,
                                 show_tempoSet_bar_pos_y);
            show_tempoAdd_bar_val = getTempo(p.X);
        }
        if (ImGui::Selectable("删除速度")) {
            show_tempoSet_bar = false;
            clickToRemoveTempo(show_tempoSet_bar_pos_x,
                               show_tempoSet_bar_pos_y);
        }
        ImGui::End();
    }
    if (show_tempoAdd_bar) {
        ImGui::Begin("设置速度", &show_tempoAdd_bar, ImGuiWindowFlags_AlwaysAutoResize);
        checkfocus();
        ImGui::InputInt("bpm", &show_tempoAdd_bar_val);
        if (show_tempoAdd_bar_val < 1) {
            show_tempoAdd_bar_val = 1;
        }
        if (show_tempoAdd_bar_val > 1024) {
            show_tempoAdd_bar_val = 1024;
        }
        if (ImGui::Button("设置")) {
            show_tempoAdd_bar = false;
            if (show_tempoAdd_bar_val > 0) {
                if (addTempo(show_tempoSet_bar_pos_x, show_tempoAdd_bar_val)) {
                    auto hisptr = std::make_shared<mgnr::history>();  //插入历史记录
                    hisptr->method = mgnr::history::H_TEMPO_ADD;
                    hisptr->tempo = show_tempoAdd_bar_val;
                    hisptr->begin = show_tempoSet_bar_pos_x;
                    pushHistory(hisptr);
                }
            }
        }
        ImGui::End();
    }
    if (show_midiDescription_bar) {
        ImGui::SetNextWindowPos(
            ImVec2(show_midiDescription_bar_pos_x,
                   show_midiDescription_bar_pos_y));

        ImGui::Begin("设置文本", &show_midiDescription_bar,
                     ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoBringToFrontOnFocus);
        checkfocus();
        if (!(ImGui::IsItemFocused() || ImGui::IsWindowFocused())) {
            show_midiDescription_bar = false;
        }
        if (ImGui::Selectable("添加文本")) {
            show_midiDescription_bar = false;
            clickToAddDescription(
                show_midiDescription_bar_pos_x,
                show_midiDescription_bar_pos_y,
                [&](int tick) {
                    show_descriptionAdd_bar = true;
                    bzero(midiDescriptionBuffer_content,
                          sizeof(midiDescriptionBuffer_content));
                    midiDescriptionBuffer_title = defaultInfo.value();
                    midiDescriptionBuffer_tick = tick;
                });
        }
        if (ImGui::Selectable("编辑文本")) {
            show_midiDescription_bar = false;
            clickToSetDescription(
                show_midiDescription_bar_pos_x,
                show_midiDescription_bar_pos_y,
                [&](int tick,
                    const mgnr::stringPool::stringPtr& title,
                    const std::string& content) {
                    snprintf(midiDescriptionBuffer_content,
                             sizeof(midiDescriptionBuffer_content),
                             "%s", content.c_str());
                    midiDescriptionBuffer_title = title.value();
                    midiDescriptionBuffer_tick = tick;
                    show_descriptionEdit_bar = true;
                });
        }
        if (ImGui::Selectable("删除文本")) {
            show_midiDescription_bar = false;
            clickToSetDescription(
                show_midiDescription_bar_pos_x,
                show_midiDescription_bar_pos_y,
                [&](int tick,
                    const mgnr::stringPool::stringPtr& title,
                    const std::string& content) {
                    delDescription(title, tick);
                });
        }
        ImGui::End();
    }
    if (show_descriptionEdit_bar) {
        ImGui::Begin("编辑文本",
                     &show_descriptionEdit_bar,
                     ImGuiWindowFlags_AlwaysAutoResize);
        checkfocus();
        ImGui::Text("音轨：%s", midiDescriptionBuffer_title.c_str());
        ImGui::InputText("设置文本",
                         midiDescriptionBuffer_content,
                         sizeof(midiDescriptionBuffer_content));
        if (ImGui::Button("保存")) {
            show_descriptionEdit_bar = false;
            addDescription(strPool.create(midiDescriptionBuffer_title),
                           midiDescriptionBuffer_tick,
                           midiDescriptionBuffer_content);
        }
        ImGui::End();
    }
    if (show_descriptionAdd_bar) {
        ImGui::Begin("添加文本",
                     &show_descriptionAdd_bar,
                     ImGuiWindowFlags_AlwaysAutoResize);
        checkfocus();
        ImGui::Text("音轨：%s", midiDescriptionBuffer_title.c_str());
        ImGui::InputText("设置文本",
                         midiDescriptionBuffer_content,
                         sizeof(midiDescriptionBuffer_content));
        if (ImGui::Button("保存")) {
            show_descriptionAdd_bar = false;
            addDescription(strPool.create(midiDescriptionBuffer_title),
                           midiDescriptionBuffer_tick,
                           midiDescriptionBuffer_content);
        }
        ImGui::End();
    }

    if (!selected.empty() && show_edit_window) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth / 2, windowHeight / 2), ImGuiCond_FirstUseEver);
        ImGui::Begin("编辑音符",
                     nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoFocusOnAppearing);
        checkfocus();

        ImGui::Text("改变时长");
        ImGui::SameLine();
        if (ImGui::Button("-")) {
            resizeSelected(-1);
        }
        ImGui::SameLine();
        if (ImGui::Button("+")) {
            resizeSelected(1);
        }

        if (ImGui::Button("复制")) {
            copy();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("ctrl+c");
        }

        ImGui::End();
    }
    if (show_trackMap_window) {
        ImGui::SetNextWindowSize(ImVec2(615, 478), ImGuiCond_FirstUseEver);
        ImGui::Begin("音轨映射表", &show_trackMap_window);
        checkfocus();
        bool selectIns = false;
        bool setTrack = false;
        int setTrack_id;
        int setTrack_ins;
        if (trackMapBuffer_closeWithSave) {
            ImGui::TextWrapped(
                "如保存midi时弹出此窗口，"
                "表明有部分音轨未分配midi轨道，"
                "系统将自动分配，请确认后再保存");
        }
        ImGui::TextWrapped("此表设置的是输出的midi文件中的轨道关系，和播放时的乐器无关");
        ImGui::TextUnformatted("");
        if (ImGui::BeginTable("音轨映射表", 3)) {
            ImGui::TableNextColumn();
            ImGui::Text("音轨名称");
            ImGui::TableNextColumn();
            ImGui::Text("音轨");
            ImGui::TableNextColumn();
            ImGui::Text("乐器");
            char buf[256];
            int index = 1;
            for (auto& it : trackMapBuffer) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", std::get<0>(it).c_str());
                ImGui::TableNextColumn();
                int n = std::get<1>(it);
                snprintf(buf, sizeof(buf), "音轨%d", index);
                if (ImGui::InputInt(buf, &std::get<1>(it))) {
                    if (std::get<1>(it) < 0) {
                        std::get<1>(it) = 0;
                    }
                    if (std::get<1>(it) > 1024) {
                        std::get<1>(it) = 1024;
                    }
                    if (n != std::get<1>(it)) {
                        setTrack = true;
                        setTrack_id = std::get<1>(it);
                        setTrack_ins = std::get<2>(it);
                    }
                }
                ImGui::TableNextColumn();
                n = std::get<2>(it);
                snprintf(buf, sizeof(buf), "设置乐器%d", index);

                if (ImGui::Button(buf)) {
                    selectIns = true;
                    trackMapBuffer_setInstrument_track = std::get<1>(it);
                    trackMapBuffer_setInstrument_ins = std::get<2>(it);
                }

                ImGui::SameLine();
                if (std::get<2>(it) < 0) {
                    std::get<2>(it) = 0;
                }
                if (std::get<2>(it) > 127) {
                    std::get<2>(it) = 127;
                }
                ImGui::TextUnformatted(mgnr::instrumentName[std::get<2>(it)]);
                ++index;
            }
            ImGui::EndTable();
        }
        if (selectIns) {
            ImGui::OpenPopup("selectInstrument");
        }
        if (ImGui::BeginPopup("selectInstrument")) {
            checkfocus();
            ImGui::Text("选择乐器");
            ImGui::Separator();
            for (int i = 0; i < 128; ++i) {
                if (ImGui::Selectable(mgnr::instrumentName[i],
                                      trackMapBuffer_setInstrument_ins == i)) {
                    setTrack = true;
                    setTrack_id = trackMapBuffer_setInstrument_track;
                    setTrack_ins = i;
                }
            }
            ImGui::EndPopup();
        }
        if (setTrack) {
            for (auto& it : trackMapBuffer) {
                if (std::get<1>(it) == setTrack_id) {
                    std::get<2>(it) = setTrack_ins;
                }
            }
        }

        if (ImGui::Button("保存")) {
            show_trackMap_window = false;
            trackMapBuffer_save();
        }
        ImGui::SameLine();
        if (ImGui::Button("复位")) {
            std::map<std::string, int> tn;
            std::map<int, int> ti;
            checkTrackMapper(tn, ti);
            trackMapBuffer_init(tn, ti);
        }

        ImGui::End();
    }

    plugin_show();

    fileDialog_saveMidi.Display();
    fileDialog_loadMidi.Display();
    if (ImGui::IsAnyItemHovered() ||
        !fileDialog_loadMidi.focusCanvas ||
        !fileDialog_saveMidi.focusCanvas) {
        focusCanvas = false;
    }
    if (!show_edit_window) {
        pasteMode = false;
    }
    if (!focusCanvas) {
        displayBuffer.clear();
    }
}
void mgenner::trackMapBuffer_init(std::map<std::string, int>& trackNameMapper,
                                  std::map<int, int>& trackInsMapper) {
    trackMapBuffer.clear();
    for (auto& it : trackNameMapper) {
        std::tuple<std::string, int, int> t;
        std::get<0>(t) = it.first;
        std::get<1>(t) = it.second;
        auto sit = trackInsMapper.find(it.second);
        if (sit == trackInsMapper.end()) {
            std::get<2>(t) = 0;
        } else {
            std::get<2>(t) = sit->second;
        }
        trackMapBuffer.push_back(std::move(t));
    }
}
void mgenner::trackMapBuffer_save() {
    trackNameMapper.clear();
    trackInsMapper.clear();
    for (auto& it : trackMapBuffer) {
        trackNameMapper[std::get<0>(it)] = std::get<1>(it);
        trackInsMapper[std::get<1>(it)] = std::get<2>(it);
    }
    if (trackMapBuffer_closeWithSave) {
        trackMapBuffer_closeWithSave = false;
        saveMidiFile();
    }
}
void mgenner::loadMidiDialog() {
    fileDialog_loadMidi.SetTypeFilters({".mid"});
    fileDialog_loadMidi.SetPwd("./");
    fileDialog_loadMidi.Open();
}
void mgenner::saveMidiDialog() {
    fileDialog_saveMidi.SetTypeFilters({".mid"});
    fileDialog_saveMidi.SetPwd("./");
    fileDialog_saveMidi.Open();
}

void mgenner::loadMidiFile(const std::string& path) {
    midiFilePath = path;
    updateWindowTitle();
    loadMidi(path);
}
void mgenner::saveMidiFile(const std::string& path) {
    midiFilePath = path;
    updateWindowTitle();
    std::map<std::string, int> tn = trackNameMapper;
    std::map<int, int> ti = trackInsMapper;
    if (!checkTrackMapper(tn, ti)) {
        show_trackMap_window = true;
        trackMapBuffer_closeWithSave = true;
        //checkTrackMapper();
        trackMapBuffer_init(tn, ti);
    } else {
        exportMidiWithTrackMapper(path);
    }
}
void mgenner::saveMidiFile() {
    if (!midiFilePath.empty()) {
        saveMidiFile(midiFilePath);
    } else {
        saveMidiDialog();
    }
}
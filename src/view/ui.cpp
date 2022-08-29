#include "mgenner.h"

void mgenner::ui() {
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("播放控制", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        CHECK_FOCUS;

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
        ImGui::SameLine();
        if (ImGui::Button("载入")) {
            loadMidiDialog();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("ctrl+o");
        }
        ImGui::SameLine();
        if (ImGui::Button("保存")) {
            saveMidiFile();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("ctrl+s");
        }
        ImGui::SameLine();
        if (ImGui::Button("另存为")) {
            saveMidiDialog();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("ctrl+shift+s");
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
        CHECK_FOCUS;

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

        if (ImGui::InputText("音轨命名", defaultInfoBuffer, sizeof(defaultInfoBuffer))) {
            if (strlen(defaultInfoBuffer) <= 1) {
            } else {
                setInfo(defaultInfoBuffer);
                show_trackSelect_window = true;
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("改变音轨命名将同时改变所有已选中的音符");
        }

        int sec = section;
        if (ImGui::InputInt("节奏", &sec)) {
            setSection(sec);
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
        CHECK_FOCUS;

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
        CHECK_FOCUS;
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
        CHECK_FOCUS;
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
        CHECK_FOCUS;
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
        CHECK_FOCUS;
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
        CHECK_FOCUS;
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
        ImGui::SetNextWindowPos(ImVec2(240, 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("编辑音符",
                     nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoFocusOnAppearing);
        CHECK_FOCUS;

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
    fileDialog_saveMidi.Display();
    fileDialog_loadMidi.Display();
    if (ImGui::IsAnyItemHovered() || !fileDialog_loadMidi.focusCanvas) {
        focusCanvas = false;
    }
    if (!show_edit_window) {
        pasteMode = false;
    }
    if (!focusCanvas) {
        displayBuffer.clear();
    }
}
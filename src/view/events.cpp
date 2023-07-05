#include "mgenner.h"

void renderContext::processEvents_mouse() {
    if (resizeNoteReady || resizeNotehover || resizeNoteMode) {
        ImGui::SetMouseCursor(4);
    } else {
        ImGui::SetMouseCursor(0);
    }
    if (drawing == nullptr) {
        return;
    }
    for (auto& event : events) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {  //按键

            if (event.motion.y < 40 + menuHeight) {  //小于40+menuHeight是marker
                if (SDL_BUTTON_RIGHT == event.button.button &&
                    drawing->show_edit_window &&
                    !show_midiDescription_bar &&
                    !show_descriptionAdd_bar &&
                    !show_descriptionEdit_bar) {
                    show_midiDescription_bar_pos_x = event.motion.x;
                    show_midiDescription_bar_pos_y = event.motion.y;
                    show_midiDescription_bar = true;
                }
            } else if (event.motion.y > windowHeight - 60) {
                if (event.motion.y < windowHeight - 30) {
                    //速度条
                    if (drawing->show_edit_window && !show_tempoAdd_bar && !show_tempoSet_bar) {
                        if (SDL_BUTTON_LEFT == event.button.button) {
                            //clickToSetTempo(event.motion.x, event.motion.y);
                        } else if (SDL_BUTTON_RIGHT == event.button.button) {
                            show_tempoSet_bar_pos_x = event.motion.x;
                            show_tempoSet_bar_pos_y = event.motion.y;
                            show_tempoSet_bar = true;
                        }
                    }
                } else {
                    //时间条
                    if (SDL_BUTTON_LEFT == event.button.button) {
                        drawing->clickToLookAt(event.motion.x, event.motion.y);
                    }
                }
            } else if (SDL_BUTTON_LEFT == event.button.button) {
                if (selectByBox) {
                    selectingByBox = true;
                    selectBoxX = event.motion.x;
                    selectBoxY = event.motion.y;
                    selectBoxXend = event.motion.x;
                    selectBoxYend = event.motion.y;
                } else {
                    if (drawing->inNote(event.motion.x, event.motion.y)) {
                        drawing->showDisplayBuffer = false;
                        if (drawing->clickToSelect(event.motion.x, event.motion.y) == 0) {
                            if (resizeNotehover) {
                                resizeNoteReady = true;
                            }
                            selectNoteFail = true;
                            moveNoteX = event.motion.x;
                            moveNoteY = event.motion.y;
                        }
                    } else {
                        if (drawing->show_edit_window) {
                            addNoteMode = true;
                        }
                    }
                }
            } else if (SDL_BUTTON_RIGHT == event.button.button) {
                drawing->clearSelected();
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (SDL_BUTTON_LEFT == event.button.button) {
                if (selectingByBox) {
                    selectingByBox = false;
                    {
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
                        auto pb = drawing->screenToAbs(bx, by);
                        auto pe = drawing->screenToAbs(ex, ey);
                        drawing->find(
                            pb, pe, [](mgnr::note* n, void* arg) {  //调用HBB搜索
                                auto self = (renderContext*)arg;
                                if (!n->selected) {  //未选择就加上选择
                                    self->drawing->selected.insert(n);
                                    n->selected = true;
                                }
                            },
                            this);
                    }
                } else {
                    if (drawing->show_edit_window) {
                        if (moveNoteMode) {
                            drawing->moveNoteEnd(event.motion.x, event.motion.y);
                        } else if (resizeNoteMode) {
                            drawing->scaleNoteEnd();
                        } else if (addNoteMode) {
                            drawing->addDisplaied();
                        } else if (selectNoteFail) {
                            drawing->clickToUnselect(event.motion.x, event.motion.y);
                        }
                    }
                }
                addNoteMode = false;
                moveNoteMode = false;
                selectNoteFail = false;
                resizeNoteMode = false;
                resizeNoteReady = false;
                drawing->showDisplayBuffer = true;
            }
        } else if (event.type == SDL_MOUSEMOTION) {  //移动鼠标
            //printf("mouse:%d %d\n", event.motion.x, event.motion.y);
            drawing->clickToDisplay(event.motion.x, event.motion.y);
            if (selectingByBox) {
                selectBoxXend = event.motion.x;
                selectBoxYend = event.motion.y;
            } else {
                if (moveNoteMode) {
                    drawing->moveNoteUpdate(event.motion.x, event.motion.y);
                } else if (resizeNoteMode) {
                    drawing->scaleNoteUpdate(event.motion.x - moveNoteX);
                } else {
                    if (selectNoteFail) {
                        if (abs(event.motion.x - moveNoteX) > 10 ||
                            abs(event.motion.y - moveNoteY) > 10) {
                            if (resizeNoteReady) {
                                resizeNoteMode = true;
                                drawing->scaleNoteBegin();
                            } else {
                                moveNoteMode = true;
                                drawing->moveNoteBegin(event.motion.x, event.motion.y);
                            }
                        }
                    }
                }
                //检测鼠标位置，确定鼠标形态
                {
                    resizeNotehover = false;
                    if (drawing->show_edit_window) {
                        auto p = drawing->screenToAbs(mouse_x, mouse_y);
                        drawing->find(
                            p, [](mgnr::note* n, void* arg) {  //调用HBB搜索
                                auto self = (renderContext*)arg;
                                if (n->selected) {
                                    auto endline = (n->begin + n->duration - self->drawing->lookAtX) * self->drawing->noteLength;
                                    if (fabs(endline - self->mouse_x) < 10) {
                                        self->resizeNotehover = true;
                                    }
                                }
                            },
                            this);
                    }
                }
            }
        } else if (event.type == SDL_MOUSEWHEEL) {
            if (mouse_y > windowHeight - 30) {
            } else if (mouse_y > windowHeight - 60) {
            } else if (mouse_y > 40) {
                if (mouse_x > 30) {
                    if (button_ctrl) {
                        if (event.wheel.y < 0) {
                            drawing->noteLength /= 1.3;
                        } else if (event.wheel.y > 0) {
                            drawing->noteLength *= 1.3;
                        }
                    } else {
                        if (event.wheel.y < 0) {
                            drawing->lookAtX += 200 / drawing->noteLength;
                        } else if (event.wheel.y > 0) {
                            drawing->lookAtX -= 200 / drawing->noteLength;
                        }
                    }
                } else {
                    if (button_ctrl) {
                        if (event.wheel.y < 0) {
                            drawing->noteHeight /= 1.3;
                        } else if (event.wheel.y > 0) {
                            drawing->noteHeight *= 1.3;
                        }
                    } else {
                        if (event.wheel.y < 0) {
                            drawing->lookAtY -= 50 / drawing->noteHeight;
                        } else if (event.wheel.y > 0) {
                            drawing->lookAtY += 50 / drawing->noteHeight;
                        }
                    }
                }
            } else {
            }
        } else if (event.type == SDL_KEYUP) {
        }
        if (event.type == SDL_KEYDOWN) {
        }
    }
}

void renderContext::processEvents_keyboard() {
    if (drawing == nullptr) {
        return;
    }
    for (auto& event : events) {
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    drawing->lookAtY += 0.3;

                    break;
                case SDLK_DOWN:
                    drawing->lookAtY -= 0.3;

                    break;
                case SDLK_LEFT:
                    drawing->lookAtX -= 3 / drawing->noteLength;

                    break;
                case SDLK_RIGHT:
                    drawing->lookAtX += 3 / drawing->noteLength;

                    break;
                case SDLK_DELETE:
                case SDLK_BACKSPACE:
                    if (drawing->show_edit_window) {
                        drawing->removeSelected();
                    }
                    break;
                case SDLK_a:
                    if (button_ctrl) {
                        drawing->selectAll();
                    }
                    break;
                case SDLK_s:
                    if (button_ctrl) {
                        if (button_shift) {
                            saveMidiDialog();
                        } else {
                            saveMidiFile();
                        }
                    } else {
                        selectByBox = !selectByBox;
                    }
                    break;
                case SDLK_c:
                    drawing->copy();
                    break;
                case SDLK_v:
                    drawing->pasteMode = true;
                    selectByBox = false;
                    break;
                case SDLK_z:
                    if (button_ctrl) {
                        if (button_shift) {
                            drawing->redo();
                        } else {
                            drawing->undo();
                        }
                    }
                    break;
                case SDLK_q:
                    drawing->noteLength /= 2;
                    break;
                case SDLK_w:
                    drawing->noteLength *= 2;
                    break;
                case SDLK_o:
                    if (button_ctrl) {
                        openMidiDialog();
                    }
                    break;
                case SDLK_l:
                    if (button_ctrl) {
                        loadMidiDialog();
                    }
                    break;
                case SDLK_n:
                    if (button_ctrl) {
                        createWindow();
                    }
                    break;
                case SDLK_EQUALS:
                    if (drawing->show_edit_window) {
                        drawing->resizeSelected(1);
                    }
                    break;
                case SDLK_MINUS:
                    if (drawing->show_edit_window) {
                        drawing->resizeSelected(-1);
                    }
                    break;
                case SDLK_SPACE:
                    if (!drawing->playingStatus) {
                        drawing->playStart();

                    } else {
                        drawing->playStop();
                    }
                    break;
            }
        }
    }
}

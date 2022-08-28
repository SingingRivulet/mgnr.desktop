#include "mgenner.h"

void mgenner::processEvents() {
    for (auto& event : events) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {  //按键

            if (event.motion.y < 40 && SDL_BUTTON_RIGHT == event.button.button) {  //小于40是marker
                if (show_edit_window &&
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
                    if (show_edit_window && !show_tempoAdd_bar && !show_tempoSet_bar) {
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
                        clickToLookAt(event.motion.x, event.motion.y);
                    }
                }
            } else if (SDL_BUTTON_LEFT == event.button.button) {
                if (selectByBox) {
                    selectingByBox = true;
                    selectBoxX = event.motion.x;
                    selectBoxY = event.motion.y;
                    selectBoxXend = event.motion.x;
                    selectBoxYend = event.motion.y;
                } else if (clickToSelect(event.motion.x, event.motion.y) <= 0 && show_edit_window)
                    addDisplaied();
            } else if (SDL_BUTTON_RIGHT == event.button.button) {
                clearSelected();
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (selectingByBox && SDL_BUTTON_LEFT == event.button.button) {
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
                    auto pb = screenToAbs(bx, by);
                    auto pe = screenToAbs(ex, ey);
                    find(
                        pb, pe, [](mgnr::note* n, void* arg) {  //调用HBB搜索
                            auto self = (mgenner*)arg;
                            if (!n->selected) {  //未选择就加上选择
                                self->selected.insert(n);
                                n->selected = true;
                            }
                        },
                        this);
                }
            }
        } else if (event.type == SDL_MOUSEMOTION) {  //移动鼠标
            mouse_x = event.motion.x;
            mouse_y = event.motion.y;
            clickToDisplay(event.motion.x, event.motion.y);
            if (selectingByBox) {
                selectBoxXend = event.motion.x;
                selectBoxYend = event.motion.y;
            }
        } else if (event.type == SDL_MOUSEWHEEL) {
            if (mouse_y > windowHeight - 30) {
                if (button_ctrl) {
                    if (event.wheel.y < 0) {
                        noteLength /= 1.3;
                    } else if (event.wheel.y > 0) {
                        noteLength *= 1.3;
                    }
                } else {
                    if (event.wheel.y < 0) {
                        lookAtX += noteLength * 20;
                    } else if (event.wheel.y > 0) {
                        lookAtX -= noteLength * 20;
                    }
                }
            } else if (mouse_y > windowHeight - 60) {
            } else if (mouse_y > 40) {
                if (button_ctrl) {
                    if (event.wheel.y < 0) {
                        noteHeight /= 1.3;
                    } else if (event.wheel.y > 0) {
                        noteHeight *= 1.3;
                    }
                } else {
                    if (event.wheel.y < 0) {
                        lookAtY -= 0.07 * noteHeight;
                    } else if (event.wheel.y > 0) {
                        lookAtY += 0.07 * noteHeight;
                    }
                }
            } else {
            }
        } else if (event.type == SDL_KEYUP) {
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    lookAtY += 0.3;

                    break;
                case SDLK_DOWN:
                    lookAtY -= 0.3;

                    break;
                case SDLK_LEFT:
                    lookAtX -= 3 / noteLength;

                    break;
                case SDLK_RIGHT:
                    lookAtX += 3 / noteLength;

                    break;
                case SDLK_DELETE:
                case SDLK_BACKSPACE:
                    if (show_edit_window) {
                        removeSelected();
                    }
                    break;
                case SDLK_a:
                    if (button_ctrl) {
                        selectAll();
                    }
                    break;
                case SDLK_s:
                    selectByBox = !selectByBox;
                    break;
                case SDLK_c:
                    copy();
                    break;
                case SDLK_v:
                    pasteMode = true;
                    break;
                case SDLK_z:
                    if (button_ctrl) {
                        if (button_shift) {
                            redo();
                        } else {
                            undo();
                        }
                    }
                    break;
                case SDLK_q:
                    noteLength /= 2;
                    break;
                case SDLK_w:
                    noteLength *= 2;
                    break;
                case SDLK_EQUALS:
                    if (show_edit_window) {
                        resizeSelected(1);
                    }
                    break;
                case SDLK_MINUS:
                    if (show_edit_window) {
                        resizeSelected(-1);
                    }
                    break;
            }
        }
    }
}
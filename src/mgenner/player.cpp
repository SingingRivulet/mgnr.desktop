#include "player.h"
namespace mgnr {

player::player() {
    tempo = 120;
    TPQ = 120;
    playingStatus = false;
    playTimes = 0;
}

void player::playStep() {
    if (playingStatus) {
        playTimes++;

        double tp = getTempo(lookAtX);
        if (round(tp - tempo) != 0) {
            tempo = tp;
            printf("mgenner:set tempo:%f\n", tempo);
        }

        //toPlay.clear();
        //获取所有此时刻的音符
        find(
            lookAtX, [](note* n, void* arg) {
                auto self = (player*)arg;
                self->noteOn(n);
            },
            this);

        //扫描没播放的音符
        std::list<note*> stopList;
        for (auto it : playing) {
            if (it->playTimes != playTimes && it != &previewNote) {  //playTimes没更新，说明音符停止
                stopList.push_back(it);
            }
        }
        for (auto it : stopList) {
            noteOff(it);
        }
        for (auto it : toPlay) {
            midiSynthesizer.play_noteOn(it);
        }
        toPlay.clear();

        //位置前移
        goNextStep();
    }
}
void player::playStop() {
    std::list<note*> stopList;
    for (auto it : playing) {
        stopList.push_back(it);
    }
    for (auto it : stopList) {
        noteOff(it);
    }
    if (previewNote.playing) {
        previewNote_off();
    }
    midiSynthesizer.play_stop();
    playing.clear();
    playingStatus = false;
}
void player::playStart() {
    for (auto it : notes) {
        it->playing = false;
    }
    playing.clear();
    lastTime = getTime();
    playingStatus = true;
    midiSynthesizer.play_start();
}

void player::previewNote_on(int pitch, int v) {
    if (previewNote.playing) {
        if (previewNote.tone != pitch) {
            noteOff(&previewNote);
        } else {
            return;
        }
    }
    previewNote.tone = pitch;
    previewNote.info = defaultInfo;
    previewNote.volume = v;
    if (playingStatus) {
        noteOn(&previewNote);
    } else {
        previewNote.playing = true;
        midiSynthesizer.play_noteOn(&previewNote);
    }
}
void player::previewNote_off() {
    previewNote.playing = false;
    noteOff(&previewNote);
}
void player::noteOn(note* n) {
    if (n) {
        if (!n->playing) {
            n->playing = true;
            playing.insert(n);
            //onNoteOn(n);
            toPlay.insert(n);
        }
        n->playTimes = playTimes;
    }
}
void player::noteOff(note* n) {
    if (n) {
        n->playing = false;
        playing.erase(n);
        midiSynthesizer.play_noteOff(n);
    }
}

long player::getTime() {
    auto n = std::chrono::system_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::milliseconds>(n.time_since_epoch());
    return d.count();
}

void player::goNextStep() {
    long tt = getTime();
    long delta = 0;
    if (lastTime > 0) {
        delta = tt - lastTime;
    }
    lastTime = tt;
    auto tps = ticksPerSecond();
    double dtick = tps * (delta / 1000.0);
    lookAtX += dtick;
}

}  // namespace mgnr

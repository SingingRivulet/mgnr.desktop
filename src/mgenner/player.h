#ifndef MGNR_PLAYER
#define MGNR_PLAYER
#include <chrono>
#include <list>
#include <set>
#include "editTable.h"
#include "synthesizer/synthesizer.h"
namespace mgnr {
class player : public editTable {
   public:
    player();
    void playStart();
    void playStop();
    void playStep();
    void noteOn(note*);
    void noteOff(note*);

    void previewNote_on(int pitch, int v);
    note previewNote;
    void previewNote_off();

    double tempo;
    virtual long getTime();
    bool playingStatus;
    std::set<note*> playing;

    synthesizer::synthesizer midiSynthesizer;

   private:
    std::set<note*> toPlay;
    long lastTime = -1;
    long playTimes;
    inline double secondsPerTick() {  //1秒=多少tick
        return 60.0 / (tempo * TPQ);
    }
    inline double ticksPerSecond() {  //1 tick=多少秒
        return (tempo * TPQ) / 60.0;
    }
    void goNextStep();
};
}  // namespace mgnr
#endif

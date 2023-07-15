#ifndef MGNR_MIDI_NOTE
#define MGNR_MIDI_NOTE
#include <string>
#include "stringPool.h"
namespace mgnr {
struct noteIndex {
    float start;
    int id;
    inline noteIndex(float s, int i) {
        start = s;
        id = i;
    }
    inline noteIndex() {
        start = 0;
        id = 0;
    }
    inline noteIndex(const noteIndex& i) {
        start = i.start;
        id = i.id;
    }
    inline const noteIndex& operator=(const noteIndex& i) {
        start = i.start;
        id = i.id;
        return *this;
    }
    inline bool operator<(const noteIndex& i) const {
        return (start < i.start && id < i.id);
    }
    inline bool operator==(const noteIndex& i) const {
        return (start == i.start && id == i.id);
    }
};
class note {
   public:
    float begin = 0;
    float tone = 0;
    float duration = 0;
    int volume = 0;
    int id = 0;
    int startId = 0;
    int endId = 0;

    stringPool::stringPtr info;

    bool selected = false;
    bool playing = false;
    long playTimes = 0;

    inline void construct() {
        selected = false;
        playing = false;
        playTimes = 0;
    }

    noteIndex beginIndex, endIndex;

    inline void getBeginIndex() {
        beginIndex = noteIndex(begin, startId);
    }
    inline void getEndIndex() {
        endIndex = noteIndex(begin + duration, endId);
    }

    void* indexer;

    note* next;  //内存池变量，不应该被修改
};
}  // namespace mgnr
#endif

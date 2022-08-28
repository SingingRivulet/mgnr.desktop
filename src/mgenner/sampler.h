#ifndef MGNR_SAMPLER
#define MGNR_SAMPLER
#include <functional>
#include <set>
#include "hbb.h"
#include "note.h"
namespace mgnr {
template <typename T>
inline void fetch(T& self, double beat, double len, const std::function<void(note*)>& callback_func) {
    int begin_tick = self.TPQ * beat;
    int len_tick = self.TPQ * len;
    struct callback_t {
        std::function<void(note*)> func;
        int begin_tick;
        int len_tick;
    } callback;
    callback.begin_tick = begin_tick;
    callback.len_tick = len_tick;
    callback.func = callback_func;
    HBB::vec from, to;
    from.X = begin_tick;
    to.X = begin_tick + len_tick;
    from.Y = 0;
    to.Y = 256;
    self.find(
        from, to, [](note* n, void* arg) {
            callback_t* callback = (callback_t*)arg;
            callback->func(n);
        },
        &callback);
}
template <typename T>
inline void fetch(T& self, double beat, const std::function<void(note*)>& callback_func) {
    int begin_tick = self.TPQ * beat;
    struct callback_t {
        std::function<void(note*)> func;
        int begin_tick;
    } callback;
    callback.begin_tick = begin_tick;
    callback.func = callback_func;
    self.find(
        begin_tick, [](note* n, void* arg) {
            callback_t* callback = (callback_t*)arg;
            callback->func(n);
        },
        &callback);
}
//采样一个音符
template <typename T>
inline int sample(T& self, double beat, double len, int base, const std::function<bool(note*)>& filter) {
    int res = 0;
    fetch(self, beat, len, [&](note* n) {
        if (filter(n)) {
            auto r = n->tone - base;
            if (r > res) {
                res = r;
            }
        }
    });
    return res;
}
//采样和弦
template <typename T>
inline std::set<int> sampleChord(T& self, double beat, double len, int base, const std::function<bool(note*)>& filter) {
    std::set<int> res;
    //std::cout<<"sampleChord"<<beat<<" "<<len<<std::endl;
    fetch(self, beat + len * 0.3, [&](note* n) {
        if (filter(n)) {
            int resval = n->tone - base;
            res.insert(resval);

            //std::cout<<"fetch:"<<n->begin/(double)self.TPQ<<" "<<(n->begin+n->delay)/(double)self.TPQ<<" "<<n->tone<<std::endl;
        }
    });
    return res;
}
}  // namespace mgnr
#endif
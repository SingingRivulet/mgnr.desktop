#ifndef MGNR_SYNTH
#define MGNR_SYNTH
#include <list>
#include <set>
#include <sstream>
#include <vector>
#include "lcs.h"
#include "player.h"
namespace mgnr {
struct voiceWord {
    int length;  //时长
    int tone;    //音高（-1为休止符）
    int volume;
    stringPool::stringPtr word;  //词汇
    voiceWord(int l, int t, int v, const stringPool::stringPtr& w)
        : word(w) {
        length = l;
        tone = t;
        volume = v;
    }
    voiceWord(const voiceWord& i)
        : word(i.word) {
        length = i.length;
        tone = i.tone;
        volume = i.volume;
    }
};
struct voiceTrack {
    std::list<voiceWord> words;
    int length;
    stringPool* strPool;
    void pushPause(int len);
    void pushNote(int len, const stringPool::stringPtr& info, int tone, int v);
    voiceTrack(stringPool* sp) {
        strPool = sp;
        length = 0;
        words.clear();
    }
};
class synth : public player {
   public:
    void onUseInfo(const stringPool::stringPtr& info);

    void synthOutput();  //链接外部引擎合成

    void toHashSeries(std::vector<std::pair<int, int> >& out);
    void diff(const std::string& out, std::function<void(int)> const& callback_d, std::function<void(int, int, int, int)> const& callback_s);
    void diff(const std::vector<std::pair<int, int> >& out, std::function<void(int)> const& callback_d, std::function<void(int, int, int, int)> const& callback_s);
    void diff_path(const std::string& path, std::function<void(int)> const& callback_d, std::function<void(int, int, int, int)> const& callback_s);
    void toHashSeries(std::string& out);
    void toThemesTrain(std::string& out, int delta);    //生成主旋律预测训练数据（供outTunner使用）
    void toThemesPredict(std::string& out, int delta);  //生成主旋律预测数据（供outTunner使用）

    std::vector<voiceTrack*> tracks;
    virtual void onLoadName(const stringPool::stringPtr& name) = 0;

    ~synth();

   private:
    void splitTracks();  //将midimap分解成音轨
    void clearTracks();

    void synthOutput_init(int);
    void synthOutput_addWord(int channel, const stringPool::stringPtr& word, int tone, int volume, float sec);
    void synthOutput_addPause(int channel, float sec);
    void synthOutput_addIntoChannel(int channel, const voiceTrack&);
    void synthOutput_start();
};

template <typename T = uint32_t, T k = 0x7FFFFFFF>
inline T hash(const char* str) {
    T s = 131;  // 31 131 1313 13131 131313 etc..
    T h = 0;

    while (*str) {
        h = h * s + (*str++);
    }

    return (h & k);
}
}  // namespace mgnr
#endif

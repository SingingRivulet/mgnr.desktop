#ifndef MGNR_MIDI_MAP
#define MGNR_MIDI_MAP
#include "hbb.h"
#include "note.h"
#include <string>
#include <functional>
#include <set>
#include <map>
#include <unordered_map>
#include <tuple>
#include <vector>
#include <algorithm>
#include <sstream>
namespace mgnr{
    class midiMap{
        public:
            midiMap();
            ~midiMap();
            stringPool strPool;
            note * addNote(float position,float tone,float delay,int v,const stringPool::stringPtr & info,int id_set=-1);
            void removeNote(note * p,bool unselect=true);
            void resizeNote(note * p);

            virtual void onUseInfo(const stringPool::stringPtr & info)=0;

            void clear();

            int find(const HBB::vec & from,const HBB::vec & to,void(*callback)(note*,void*),void * arg);
            int find(float step,void(*callback)(note*,void*),void * arg);
            int find(const HBB::vec & p,void(*callback)(note*,void*),void * arg);
            //获取范围内的音符

            stringPool::stringPtr infoFilter;
            std::set<note*> notes;
            std::map<noteIndex,note*> timeIndex;

            std::map<int,double> timeMap;

            std::set<note*> selected;

            virtual void onSelectedChange(int len)=0;
            inline void updateSelectedStatus(){
                onSelectedChange(selected.size());
            }

            int TPQ;
            float XShift;
            virtual void rebuildNoteLen()=0;

            float sectionLen;//小节线
            int section;

            inline void setSection(int s){
                section = s;
                setSection();
            }
            inline void setSection(){
                if(section<=0 || section>7)
                    section=4;
                sectionLen=section*TPQ;
            }

            double getTempo(int tick);
            bool addTempo(int tick,double tp);
            std::tuple<bool,int,double> removeTempoBeforePos(int tick);
            void getTempo(int begin,const std::function<bool(int,double)> & callback);//获取一段区域的速度

            void removeControl(float begin,const stringPool::stringPtr & info);
            void addControl(float begin,const stringPool::stringPtr & info);

            int getAreaNote(float begin,float len,const std::string & info,float forceLen=0.6666,float minLen=0.25);//获取一个范围内的主要音符
            int getSectionNote(float sec,const std::string & info,float forceLen=0.6666,float minLen=0.25);

            inline void removeNoteById(int id){
                auto p = seekNoteById(id);
                if(p){
                    removeNote(p);
                }
            }

            inline note * seekNoteById(int id){
                auto it = noteIDs.find(id);
                if(it!=noteIDs.end()){
                    return it->second;
                }
                return NULL;
            }

            std::unordered_map<std::string,std::vector<int> > chord_map;
            std::unordered_map<std::string,int> note_number_map;
            std::map<std::string,int> chord_map_note;

            //文本信息
            std::map<stringPool::stringPtr,std::map<int,std::string>> descriptions;
            //添加文本
            inline void
            addDescription(const stringPool::stringPtr & key,int tm,const std::string & val){
                descriptions[key][tm] = val;
            }
            //获取文本
            inline void
            getDescription(const stringPool::stringPtr & key,int tm,
                           const std::function<void(const std::string &)> & callback) {
                auto it_key = descriptions.find(key);
                if (it_key != descriptions.end()) {
                    auto it_tm = it_key->second.find(tm);
                    if (it_tm != it_key->second.end()) {
                        callback(it_tm->second);
                    }
                }
            }
            //删除文本
            inline void
            delDescription(const stringPool::stringPtr & key,int tm) {
                auto it_key = descriptions.find(key);
                if (it_key != descriptions.end()) {
                    auto it_tm = it_key->second.find(tm);
                    if (it_tm != it_key->second.end()) {
                        it_key->second.erase(it_tm);
                    }
                    if (it_key->second.empty()) {
                        descriptions.erase(it_key);
                    }
                }
            }
            //搜索文本
            inline void
            findDescription(const std::map<int,std::string> & obj,
                            int tm_begin,int tm_end,
                            const std::function<void(int,const std::string&)> & callback) {
                auto it = obj.upper_bound(tm_begin);//获取大于tick的第一个元素
                while (it != obj.end() && it->first <= tm_end) {
                    callback(it->first,it->second);
                }
            }
            inline void
            findDescription(const stringPool::stringPtr & target,
                        int tm_begin,int tm_end,
                        const std::function<void(int,const std::string&)> & callback) {
                auto it = descriptions.find(target);
                if (it != descriptions.end()) {
                    findDescription(it->second, tm_begin, tm_end, callback);
                }
            }
            inline void
            findDescription(int tm_begin,int tm_end,
                            const std::function<void(int,const std::string&)> & callback) {
                for (auto &it:descriptions) {
                    findDescription(it.second, tm_begin, tm_end, callback);
                }
            }

            float noteTimeMax;
            float noteToneMax;
            float noteToneMin;
            bool noteUpdated;
            bool updateTimeMax();
        private:
            std::map<int,note*> noteIDs;
            HBB indexer;
            void * pool;
            int id=0;
    };
}
#endif

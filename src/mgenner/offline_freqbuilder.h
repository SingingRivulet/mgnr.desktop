//
// Created by admin on 2022/7/9.
//

#ifndef MIDILIB_OFFLINE_FREQBUILDER_H
#define MIDILIB_OFFLINE_FREQBUILDER_H

#include "freq.hpp"
#include "offline.h"

namespace mgnr::offline_render {

inline void render(const std::string& midi,
                   const char* sf,
                   int sampleRate,
                   const std::function<void(const float*, mgnr::offline&)>& callback) {
    printf("offline_render:render");
    mgnr::offline renderer(sf, sampleRate);
    renderer.loadMidi(midi);
    float buf[512];
    printf("offline_render:render start...");
    renderer.updateTimeMax();
    while (renderer.renderStep(buf)) {
        callback(buf, renderer);
    }
}

inline void getFreq(const std::string& midi,
                    const char* sf,
                    int sampleRate,
                    int fftSize,
                    int skip,      //处理时跳过帧
                    int blockNum,  //每一次有多少个block，剩下的补0
                    const std::function<void(const sinrivUtils::cmplx*,
                                             mgnr::offline&)>& callback) {
    if (blockNum <= 0) {
        return;
    }

    printf("offline_render:getFreq");
    sinrivUtils::cmplx fftBuffer[fftSize];
    sinrivUtils::cmplx fftBuffer_out[fftSize];
    float window[fftSize];
    //窗函数
    for (int i = 0; i < fftSize; ++i) {
        window[i] = (1.0 - 0.46) - (0.46 * cos(2.0 * M_PI * i / (fftSize - 1.0)));
    }

    int dataNum = blockNum * 512;
    if (dataNum <= fftSize) {
        //计算左右的空白
        int padLeft, padRight;
        if (dataNum == fftSize) {
            padLeft = 0;
            padRight = 0;
        } else {
            padLeft = (fftSize - dataNum) / 2;
            padRight = fftSize - padLeft - dataNum;
        }
        //构建数组缓冲
        std::list<float*> dataBuff;
        for (int i = 0; i < blockNum; ++i) {
            auto ptr = new float[512];
            for (int j = 0; j < 512; ++j) {
                ptr[j] = 0;
            }
            dataBuff.push_back(ptr);
        }

        //渲染
        int frameId = 0;
        render(midi, sf, sampleRate, [&](const float* buffer, mgnr::offline& renderer) {
            //填充数组
            auto dataBuff_it = dataBuff.begin();
            if (dataBuff_it != dataBuff.end()) {
                float* ptr = *dataBuff_it;
                dataBuff.erase(dataBuff_it);
                dataBuff.push_back(ptr);
                for (int i = 0; i < 512; ++i) {
                    ptr[i] = buffer[i];
                }
            }
            if (frameId % skip == 0) {
                //构造输入
                int index = 0;
                //补充左边
                for (int i = 0; i < padLeft; ++i) {
                    fftBuffer[index].r = 0;
                    fftBuffer[index].i = 0;
                    ++index;
                }
                //填充数据
                for (auto& it : dataBuff) {
                    for (int j = 0; j < 512; ++j) {
                        fftBuffer[index].r = it[j];
                        fftBuffer[index].i = 0;
                        ++index;
                    }
                }
                //补充右边
                for (int i = 0; i < padRight; ++i) {
                    fftBuffer[index].r = 0;
                    fftBuffer[index].i = 0;
                    ++index;
                }
                //加窗
                for (int i = 0; i < fftSize; ++i) {
                    fftBuffer[i].r *= window[i];
                }
                //执行变换
                sinrivUtils::FFT(fftBuffer, fftBuffer_out, fftSize);
                callback(fftBuffer_out, renderer);
            }
            ++frameId;
        });

        //释放空间
        for (auto& it : dataBuff) {
            delete[] it;
        }
    }
}

inline void getMidiMap(const std::string& midi,
                       const char* sf,
                       int sampleRate,
                       int fftSize,
                       int skip,       //处理时跳过帧
                       int blockNum,   //每一次有多少个block，剩下的补0
                       float minFreq,  //频谱被标记的阈值
                       const std::function<void(const float*, const int*, mgnr::offline&)>& callback) {
    printf("offline_render:getMidiMap");
    float midiBuffer[512];
    int tag[512];
    int midiMapper_begin[512];
    int midiMapper_end[512];
    //计算midi132位置
    double midi132 = 440. * pow(2., (132. - 57.) / 12.) * fftSize / sampleRate;
    //计算映射关系
    const int idx_max = fftSize / 2 - 1;
    for (int idx = 0; idx < 512; ++idx) {
        int pos_begin = midi132 * pow(2, ((idx - 0.5) / 4.0 - 132.0) / 12.0);
        int pos_end = midi132 * pow(2, ((idx + 0.5) / 4.0 - 132.0) / 12.0);
        if (pos_begin < 0) {
            pos_begin = 0;
        }
        if (pos_end < 0) {
            pos_end = 0;
        }
        if (pos_begin > idx_max) {
            pos_begin = idx_max;
        }
        if (pos_end > idx_max) {
            pos_end = idx_max;
        }
        midiMapper_begin[idx] = pos_begin;
        midiMapper_end[idx] = pos_end;
    }
    getFreq(midi, sf, sampleRate, fftSize, skip, blockNum,
            [&](const sinrivUtils::cmplx* buffer, mgnr::offline& renderer) {
                //::__android_log_print(ANDROID_LOG_INFO, "offline_render","render freq map");
                bzero(midiBuffer, sizeof(midiBuffer));
                bzero(tag, sizeof(tag));
                for (int idx = 0; idx < 512; ++idx) {
                    int begin = midiMapper_begin[idx];
                    int end = midiMapper_end[idx];
                    if (begin < idx_max) {
                        //区间中取最大的
                        float maxTone = 0;
                        for (int i = begin; i <= end; ++i) {
                            float toneAbs =
                                buffer[i].r * buffer[i].r + buffer[i].i * buffer[i].i;
                            if (toneAbs > maxTone) {
                                maxTone = toneAbs;
                            }
                        }
                        midiBuffer[idx] = sqrt(maxTone);
                    }
                }
                //计算实际的频谱
                for (auto& it : renderer.playing) {
                    int begin = it->tone * 4;
                    int end = (it->tone + 1) * 4;
                    for (int i = begin; i < end; ++i) {
                        if (i >= 0 && i < 512 && midiBuffer[i] > minFreq) {
                            tag[i] = 1;  //标记
                        }
                    }
                }
                callback(midiBuffer, tag, renderer);
            });
}

inline void getMidiMap(const std::string& midi,
                       const char* sf,
                       int sampleRate,
                       int fftSize,
                       int skip,       //处理时跳过帧
                       int blockNum,   //每一次有多少个block，剩下的补0
                       float minFreq,  //频谱被标记的阈值
                       const char* outPath) {
    auto fp = fopen(outPath, "w");
    if (fp) {
        getMidiMap(midi, sf, sampleRate, fftSize, skip, blockNum, minFreq,
                   [&](const float* freq, const int* tag, mgnr::offline& renderer) {
                       fprintf(fp, "%f", renderer.lookAtX / renderer.TPQ);
                       for (int i = 0; i < 512; ++i) {
                           fprintf(fp, ",%f:%d", freq[i], tag[i]);
                       }
                       fprintf(fp, "\n");
                   });
        fclose(fp);
    }
}

}  // namespace mgnr::offline_render

#endif  //MIDILIB_OFFLINE_FREQBUILDER_H

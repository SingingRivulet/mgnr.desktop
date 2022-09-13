#pragma once
#include <memory>
#include "WavFile.h"
#include "freq.hpp"
#include "view/mgenner.h"

struct audioStream_t : public mgnr::vscript::value {
    virtual bool eof() = 0;
    virtual int getNumChannel() = 0;
    virtual int getSampleRate() = 0;
};

struct wav_input_t : public audioStream_t {
    WavInFile input;
    wav_input_t(const std::string& path);
    bool eof() override;
    virtual int read(float* buffer, int maxElems);
    int getNumChannel() override;
    int getSampleRate() override;
};

//波形采样
struct wav_frame_t : public mgnr::vscript::value {
    float* buffer;
    void* buffer_p;
    int size, channel;
    wav_frame_t(int size, int channel);
    ~wav_frame_t() override;
    inline float* operator[](int index) {
        return &buffer[index * size];
    }
    inline const float* operator[](int index) const {
        return &buffer[index * size];
    }
};

//时/频谱图
struct spectrum_t : public mgnr::vscript::value {
    sinrivUtils::cmplx* buffer;
    void* buffer_p;
    int size, channel;
    spectrum_t(int size, int channel);
    ~spectrum_t() override;
    inline sinrivUtils::cmplx* operator[](int index) {
        return &buffer[index * size];
    }

    inline const sinrivUtils::cmplx* operator[](int index) const {
        return &buffer[index * size];
    }

    void FFT(spectrum_t* w) const;
    void IFFT(spectrum_t* w) const;
    void buildCepstrum(wav_frame_t* w) const;
    void window(float* w);
};

struct frameSampler_t : public audioStream_t {
    std::shared_ptr<wav_input_t> input;
    inline frameSampler_t(std::shared_ptr<wav_input_t> input) {
        this->input = input;
    }
    bool eof() override;
    int getNumChannel() override;
    int getSampleRate() override;
    virtual void read(wav_frame_t* buffer);
    virtual void read(spectrum_t* buffer);
};

struct spectrumBuilder_t : public audioStream_t {
    std::shared_ptr<frameSampler_t> input;
    inline spectrumBuilder_t(std::shared_ptr<frameSampler_t> input) {
        this->input = input;
    }
    bool eof() override;
    int getNumChannel() override;
    int getSampleRate() override;
    virtual void read(spectrum_t* buffer);
};

struct cepstrumBuilder_t : public audioStream_t {
    std::shared_ptr<spectrumBuilder_t> input;
    inline cepstrumBuilder_t(std::shared_ptr<spectrumBuilder_t> input) {
        this->input = input;
    }
    bool eof() override;
    int getNumChannel() override;
    int getSampleRate() override;
    virtual void read(wav_frame_t* buffer);
};

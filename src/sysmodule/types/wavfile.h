#pragma once
#include <functional>
#include <memory>
#include "WavFile.h"
#include "freq.hpp"
#include "view/mgenner.h"
#include "waveWindow.h"

struct audioStream_t : public mgnr::vscript::value {
    virtual int getNumChannel() = 0;
    virtual int getSampleRate() = 0;
    virtual int getNumBits() = 0;
    virtual int getFrameSize() = 0;
};

struct wav_input_t : public audioStream_t {
    WavInFile input;
    int blockSize = 256;
    wav_input_t(const std::string& path);
    virtual void read(const std::function<void(float*, int)>& callback, bool fix = true);
    int getNumChannel() override;
    int getSampleRate() override;
    int getNumBits() override;
    int getFrameSize() override;
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

struct frameStream_t : public audioStream_t {
    virtual void read(const std::function<void(wav_frame_t&)>& callback) = 0;
    virtual void read(const std::function<void(spectrum_t&)>& callback) = 0;
};

struct frameSampler_t : public frameStream_t {
    std::shared_ptr<wav_input_t> input;
    int overlap = 2;
    inline frameSampler_t(std::shared_ptr<wav_input_t> input) {
        this->input = input;
    }
    int getNumChannel() override;
    int getSampleRate() override;
    int getFrameSize() override;
    int getNumBits() override;
    virtual void read(const std::function<void(wav_frame_t&)>& callback);
    virtual void read(const std::function<void(spectrum_t&)>& callback);
};

struct frameWindow_t : public frameStream_t {
    sinrivUtils::waveWindow::SoundWindowType windowType = sinrivUtils::waveWindow::SOUND_WINDOW_HANNING;
    std::shared_ptr<frameStream_t> input;
    inline frameWindow_t(std::shared_ptr<frameStream_t> input) {
        this->input = input;
    }
    int getNumChannel() override;
    int getSampleRate() override;
    int getFrameSize() override;
    int getNumBits() override;
    virtual void read(const std::function<void(wav_frame_t&)>& callback);
    virtual void read(const std::function<void(spectrum_t&)>& callback);
};

struct spectrumBuilder_t : public audioStream_t {
    std::shared_ptr<frameStream_t> input;
    inline spectrumBuilder_t(std::shared_ptr<frameStream_t> input) {
        this->input = input;
    }
    int getNumChannel() override;
    int getSampleRate() override;
    int getFrameSize() override;
    int getNumBits() override;
    virtual void read(const std::function<void(spectrum_t&)>& callback);
};

struct cepstrumBuilder_t : public audioStream_t {
    std::shared_ptr<spectrumBuilder_t> input;
    inline cepstrumBuilder_t(std::shared_ptr<spectrumBuilder_t> input) {
        this->input = input;
    }
    int getNumChannel() override;
    int getSampleRate() override;
    int getFrameSize() override;
    int getNumBits() override;
    virtual void read(const std::function<void(wav_frame_t&)>& callback);
};

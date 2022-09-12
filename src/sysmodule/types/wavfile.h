#pragma once
#include <memory>
#include "WavFile.h"
#include "view/mgenner.h"

struct audioStream_t : public mgnr::vscript::value {
    virtual bool eof() = 0;
    virtual int read(float* buffer, int maxElems) = 0;
};

struct wav_input_t : public audioStream_t {
    WavInFile input;
    wav_input_t(const std::string& path);
    bool eof() override;
    int read(float* buffer, int maxElems) override;
};
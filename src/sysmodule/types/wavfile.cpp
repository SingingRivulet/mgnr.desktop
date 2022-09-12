#include "wavfile.h"

wav_input_t::wav_input_t(const std::string& path)
    : input(path.c_str()) {
}

bool wav_input_t::eof() {
    return input.eof() != 0;
}

int wav_input_t::read(float* buffer, int maxElems) {
    return input.read(buffer, maxElems);
}

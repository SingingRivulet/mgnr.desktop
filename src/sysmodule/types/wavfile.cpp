#include "wavfile.h"
#include "mempool.h"

using cmplx = sinrivUtils::cmplx;

#define pool_block(T)       \
    struct pool_block_##T { \
        int len;            \
        T* buffer;          \
    };

#define pool_block_imp(T, L)                              \
    struct pool_block_##T##_##L : public pool_block_##T { \
        pool_block_##T##_##L* next;                       \
        T buffer_data[L];                                 \
        pool_block_##T##_##L() {                          \
            len = L;                                      \
            buffer = buffer_data;                         \
        }                                                 \
    };

#define wavBuffer_pool(T)                                                       \
    pool_block(T);                                                              \
    pool_block_imp(T, 512);                                                     \
    pool_block_imp(T, 2048);                                                    \
    pool_block_imp(T, 8192);                                                    \
    pool_block_imp(T, 65536);                                                   \
    struct wavBuffer_pool_##T {                                                 \
        mgnr::mempool<pool_block_##T##_512> pf512;                              \
        mgnr::mempool<pool_block_##T##_2048> pf2048;                            \
        mgnr::mempool<pool_block_##T##_8192> pf8192;                            \
        mgnr::mempool<pool_block_##T##_65536> pf65536;                          \
        pool_block_##T* get(int len) {                                          \
            if (len <= 512) {                                                   \
                return pf512.get();                                             \
            } else if (len <= 2048) {                                           \
                return pf2048.get();                                            \
            } else if (len <= 8192) {                                           \
                return pf8192.get();                                            \
            } else if (len <= 65536) {                                          \
                return pf65536.get();                                           \
            } else {                                                            \
                return nullptr;                                                 \
            }                                                                   \
        }                                                                       \
        void del(pool_block_##T* b) {                                           \
            if (b) {                                                            \
                switch (b->len) {                                               \
                    case 512:                                                   \
                        pf512.del((pool_block_##T##_512*)(b));                  \
                        break;                                                  \
                                                                                \
                    case 2048:                                                  \
                        pf2048.del((pool_block_##T##_2048*)(b));                \
                        break;                                                  \
                                                                                \
                    case 8192:                                                  \
                        pf8192.del((pool_block_##T##_8192*)(b));                \
                        break;                                                  \
                                                                                \
                    case 65536:                                                 \
                        pf65536.del((pool_block_##T##_65536*)(b));              \
                        break;                                                  \
                                                                                \
                    default:                                                    \
                        printf("mgnr:release memory failed:size=%d\n", b->len); \
                        break;                                                  \
                }                                                               \
            }                                                                   \
        }                                                                       \
    }

wavBuffer_pool(float) wavBuffer_pool_f;
wavBuffer_pool(cmplx) wavBuffer_pool_c;

wav_input_t::wav_input_t(const std::string& path)
    : input(path.c_str()) {
}

bool wav_input_t::eof() {
    return input.eof() != 0;
}

int wav_input_t::read(float* buffer, int maxElems) {
    return input.read(buffer, maxElems);
}

int wav_input_t::getNumChannel() {
    return input.getNumChannels();
}

int wav_input_t::getSampleRate() {
    return input.getSampleRate();
}

wav_frame_t::wav_frame_t(int size, int channel) {
    this->size = size;
    this->channel = channel;
    auto buffer_head = wavBuffer_pool_f.get(size * channel);
    buffer = buffer_head->buffer;
    buffer_p = buffer_head;
}

wav_frame_t::~wav_frame_t() {
    wavBuffer_pool_f.del((pool_block_float*)buffer_p);
}

spectrum_t::spectrum_t(int size, int channel) {
    this->size = size;
    this->channel = channel;
    auto buffer_head = wavBuffer_pool_c.get(size * channel);
    buffer = buffer_head->buffer;
    buffer_p = buffer_head;
}

spectrum_t::~spectrum_t() {
    wavBuffer_pool_c.del((pool_block_cmplx*)buffer_p);
}

void spectrum_t::FFT(spectrum_t* w) const {
    if (w->size == size && w->channel == channel) {
        for (int channel_id = 0; channel_id < channel; ++channel_id) {
            auto ptr_in = (*this)[channel_id];
            auto ptr_out = (*w)[channel_id];
            sinrivUtils::FFT(ptr_in, ptr_out, size);
        }
    }
}

void spectrum_t::IFFT(spectrum_t* w) const {
    if (w->size == size && w->channel == channel) {
        for (int channel_id = 0; channel_id < channel; ++channel_id) {
            auto ptr_in = (*this)[channel_id];
            auto ptr_out = (*w)[channel_id];
            sinrivUtils::IFFT(ptr_in, ptr_out, size);
        }
    }
}

void spectrum_t::buildCepstrum(wav_frame_t* w) const {
    sinrivUtils::cmplx buffer[size];
    sinrivUtils::cmplx buffer_out[size];
    if (w->size == size && w->channel == channel) {
        for (int channel_id = 0; channel_id < channel; ++channel_id) {
            auto ptr_self = (*this)[channel_id];
            for (int i = 0; i < size; ++i) {
                //构建对数谱
                auto ws = sqrt(ptr_self[i].r * ptr_self[i].r + ptr_self[i].i * ptr_self[i].i);
                if (ws < 0.0001) {
                    ws = 0.0001;
                }
                buffer[i].r = log(ws);
                buffer[i].i = 0;
            }

            sinrivUtils::IFFT(buffer, buffer_out, size);

            auto ptr_out = (*w)[channel_id];
            for (int i = 0; i < size; ++i) {
                ptr_out[i] = sqrt(
                    buffer_out[i].r * buffer_out[i].r +
                    buffer_out[i].i * buffer_out[i].i);
            }
        }
    }
}

void spectrum_t::window(float* w) {
    for (int channel_id = 0; channel_id < channel; ++channel_id) {
        auto ptr = (*this)[channel_id];
        for (int i = 0; i < size; ++i) {
            ptr[i].r *= w[i];
            ptr[i].i *= w[i];
        }
    }
}

bool frameSampler_t::eof() {
    return input->eof();
}
int frameSampler_t::getNumChannel() {
    return input->getNumChannel();
}
int frameSampler_t::getSampleRate() {
    return input->getSampleRate();
}
void frameSampler_t::read(wav_frame_t* buffer) {
    int numChannel = getNumChannel();
    if (buffer->size > 0 && buffer->channel == numChannel) {
        int maxlen = buffer->size * numChannel;
        float fbuffer[maxlen];
        int readlen = input->read(fbuffer, maxlen);
        for (int i = readlen; i < maxlen; ++i) {
            fbuffer[i] = 0;
        }
        for (int channel_id = 0; channel_id < numChannel; ++channel_id) {
            auto ptr_self = (*buffer)[channel_id];
            for (int i = 0; i < buffer->size; ++i) {
                ptr_self[i] = fbuffer[i * numChannel + channel_id];
            }
        }
    }
}
void frameSampler_t::read(spectrum_t* buffer) {
    int numChannel = getNumChannel();
    if (buffer->size > 0 && buffer->channel == numChannel) {
        int maxlen = buffer->size * numChannel;
        float fbuffer[maxlen];
        int readlen = input->read(fbuffer, maxlen);
        for (int i = readlen; i < maxlen; ++i) {
            fbuffer[i] = 0;
        }
        for (int channel_id = 0; channel_id < numChannel; ++channel_id) {
            auto ptr_self = (*buffer)[channel_id];
            for (int i = 0; i < buffer->size; ++i) {
                ptr_self[i].r = fbuffer[i * numChannel + channel_id];
                ptr_self[i].i = 0;
            }
        }
    }
}

bool spectrumBuilder_t::eof() {
    return input->eof();
}
int spectrumBuilder_t::getNumChannel() {
    return input->getNumChannel();
}
int spectrumBuilder_t::getSampleRate() {
    return input->getSampleRate();
}
void spectrumBuilder_t::read(spectrum_t* buffer) {
    spectrum_t obuffer(buffer->size, buffer->channel);
    input->read(&obuffer);
    obuffer.FFT(buffer);
}

bool cepstrumBuilder_t::eof() {
    return input->eof();
}
int cepstrumBuilder_t::getNumChannel() {
    return input->getNumChannel();
}
int cepstrumBuilder_t::getSampleRate() {
    return input->getSampleRate();
}
void cepstrumBuilder_t::read(wav_frame_t* buffer) {
    spectrum_t obuffer(buffer->size, buffer->channel);
    input->read(&obuffer);
    obuffer.buildCepstrum(buffer);
}
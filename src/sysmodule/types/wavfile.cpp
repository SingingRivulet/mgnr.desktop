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

int wav_input_t::getNumBits() {
    return input.getNumBits();
}

void wav_input_t::read(const std::function<void(float*, int)>& callback, bool fix) {
    int maxElems = getNumChannel() * blockSize;
    auto pbuffer = wavBuffer_pool_f.get(maxElems);
    auto buffer = pbuffer->buffer;
    while (!input.eof()) {
        int readlen = input.read(buffer, maxElems);
        if (fix) {
            for (int i = readlen; i < maxElems; ++i) {
                buffer[i] = 0;
            }
        }
        callback(buffer, readlen);
    }
    input.rewind();
    wavBuffer_pool_f.del(pbuffer);
}

int wav_input_t::getNumChannel() {
    return input.getNumChannels();
}

int wav_input_t::getSampleRate() {
    return input.getSampleRate();
}

int wav_input_t::getFrameSize() {
    return blockSize;
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
    } else {
        throw std::runtime_error("spectrum_t：FFT：长度不匹配");
    }
}

void spectrum_t::IFFT(spectrum_t* w) const {
    if (w->size == size && w->channel == channel) {
        for (int channel_id = 0; channel_id < channel; ++channel_id) {
            auto ptr_in = (*this)[channel_id];
            auto ptr_out = (*w)[channel_id];
            sinrivUtils::IFFT(ptr_in, ptr_out, size);
        }
    } else {
        throw std::runtime_error("spectrum_t：IFFT：长度不匹配");
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
                if (ws < 1e-9) {
                    ws = 1e-9;
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
    } else {
        throw std::runtime_error("spectrum_t：构建倒频谱：长度不匹配");
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

int frameSampler_t::getNumChannel() {
    return input->getNumChannel();
}
int frameSampler_t::getSampleRate() {
    return input->getSampleRate();
}
int frameSampler_t::getFrameSize() {
    return input->getFrameSize() * overlap;
}
int frameSampler_t::getNumBits() {
    return input->getNumBits();
}
void frameSampler_t::read(const std::function<void(wav_frame_t&)>& callback) {
    int numChannel = getNumChannel();
    std::list<std::unique_ptr<wav_frame_t>> buffer;
    int singleFrameSize = input->getFrameSize();
    for (int i = 0; i < overlap; ++i) {
        buffer.push_back(
            std::move(
                std::unique_ptr<wav_frame_t>(
                    new wav_frame_t(singleFrameSize, numChannel))));  //采用未重叠的长度
    }
    wav_frame_t obuffer(getFrameSize(), numChannel);  //采用已重叠的长度
    input->read([&](float* fbuffer, int size) {
        //循环缓冲
        auto it = buffer.begin();
        std::unique_ptr<wav_frame_t> tmp = std::move(*it);
        buffer.pop_front();
        memcpy(tmp->buffer, fbuffer, sizeof(float) * size);
        buffer.push_back(std::move(tmp));
        //重组
        int blockId = 0;
        for (auto& buffer_it : buffer) {
            for (int channel_id = 0; channel_id < numChannel; ++channel_id) {
                auto ptr_self = obuffer[channel_id];
                for (int i = 0; i < buffer_it->size; ++i) {
                    ptr_self[i + blockId * buffer_it->size] = buffer_it->buffer[i * numChannel + channel_id];
                }
            }
            ++blockId;
        }
        callback(obuffer);
    });
}
void frameSampler_t::read(const std::function<void(spectrum_t&)>& callback) {
    int numChannel = getNumChannel();
    std::list<std::unique_ptr<wav_frame_t>> buffer;
    int singleFrameSize = input->getFrameSize();
    for (int i = 0; i < overlap; ++i) {
        buffer.push_back(
            std::move(
                std::unique_ptr<wav_frame_t>(
                    new wav_frame_t(singleFrameSize, numChannel))));  //采用未重叠的长度
    }
    spectrum_t obuffer(getFrameSize(), numChannel);  //采用已重叠的长度
    input->read([&](float* fbuffer, int size) {
        //循环缓冲
        auto it = buffer.begin();
        std::unique_ptr<wav_frame_t> tmp = std::move(*it);
        buffer.pop_front();
        memcpy(tmp->buffer, fbuffer, sizeof(float) * size);
        buffer.push_back(std::move(tmp));
        //重组
        int blockId = 0;
        for (auto& buffer_it : buffer) {
            for (int channel_id = 0; channel_id < numChannel; ++channel_id) {
                auto ptr_self = obuffer[channel_id];
                for (int i = 0; i < buffer_it->size; ++i) {
                    ptr_self[i + blockId * buffer_it->size].r = buffer_it->buffer[i * numChannel + channel_id];
                    ptr_self[i + blockId * buffer_it->size].i = 0;
                }
            }
            ++blockId;
        }
        callback(obuffer);
    });
}

int frameWindow_t::getNumChannel() {
    return input->getNumChannel();
}
int frameWindow_t::getSampleRate() {
    return input->getSampleRate();
}
int frameWindow_t::getFrameSize() {
    return input->getFrameSize();
}
int frameWindow_t::getNumBits() {
    return input->getNumBits();
}
void frameWindow_t::read(const std::function<void(wav_frame_t&)>& callback) {
    int numChannel = getNumChannel();
    int len = input->getFrameSize();
    auto window = wavBuffer_pool_f.get(len);
    //构建窗函数
    sinrivUtils::waveWindow::create(window->buffer, len, windowType);
    wav_frame_t buffer(len, numChannel);
    input->read([&](const wav_frame_t& w) {
        for (int channel_id = 0; channel_id < numChannel; ++channel_id) {
            auto ptr_buffer = buffer[channel_id];
            auto ptr_wave = w[channel_id];
            for (int i = 0; i < buffer.size; ++i) {
                ptr_buffer[i] = window->buffer[i] * ptr_wave[i];
            }
        }
        callback(buffer);
    });
    wavBuffer_pool_f.del(window);
}
void frameWindow_t::read(const std::function<void(spectrum_t&)>& callback) {
    int numChannel = getNumChannel();
    int len = input->getFrameSize();
    auto window = wavBuffer_pool_f.get(len);
    //构建窗函数
    sinrivUtils::waveWindow::create(window->buffer, len, windowType);
    spectrum_t buffer(input->getFrameSize(), numChannel);
    input->read([&](const spectrum_t& w) {
        for (int channel_id = 0; channel_id < numChannel; ++channel_id) {
            auto ptr_buffer = buffer[channel_id];
            auto ptr_wave = w[channel_id];
            for (int i = 0; i < buffer.size; ++i) {
                ptr_buffer[i].r = window->buffer[i] * ptr_wave[i].r;
                ptr_buffer[i].i = window->buffer[i] * ptr_wave[i].i;
            }
        }
        callback(buffer);
    });
    wavBuffer_pool_f.del(window);
}

int spectrumBuilder_t::getNumChannel() {
    return input->getNumChannel();
}
int spectrumBuilder_t::getSampleRate() {
    return input->getSampleRate();
}
int spectrumBuilder_t::getFrameSize() {
    return input->getFrameSize();
}
int spectrumBuilder_t::getNumBits() {
    return input->getNumBits();
}
void spectrumBuilder_t::read(const std::function<void(spectrum_t&)>& callback) {
    int numChannel = getNumChannel();
    spectrum_t buffer(input->getFrameSize(), numChannel);
    input->read([&](spectrum_t& obuffer) {
        obuffer.FFT(&buffer);
        callback(buffer);
    });
}

int cepstrumBuilder_t::getNumChannel() {
    return input->getNumChannel();
}
int cepstrumBuilder_t::getSampleRate() {
    return input->getSampleRate();
}
int cepstrumBuilder_t::getFrameSize() {
    return input->getFrameSize();
}
int cepstrumBuilder_t::getNumBits() {
    return input->getNumBits();
}
void cepstrumBuilder_t::read(const std::function<void(wav_frame_t&)>& callback) {
    int numChannel = getNumChannel();
    wav_frame_t buffer(input->getFrameSize(), numChannel);
    input->read([&](spectrum_t& obuffer) {
        obuffer.buildCepstrum(&buffer);
        callback(buffer);
    });
}
#pragma once
#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <sox.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory>
#include <mutex>
namespace mgnr::synthesizer {

inline void* lsx_checkptr(void* ptr) {
    if (!ptr) {
        lsx_fail("out of memory");
        exit(2);
    }

    return ptr;
}
inline void* lsx_calloc(size_t n, size_t size) {
    return lsx_checkptr(calloc(n + !n, size + !size));
}

struct streamPlayer {
    int sampleRate = 44100;
    sox_format_t* out = NULL;
    //定义音频参数
    typedef enum { RG_off,
                   RG_track,
                   RG_album,
                   RG_default } rg_mode;
    typedef struct {
        char* filename;
        /* fopts */
        char const* filetype;
        sox_signalinfo_t signal;
        sox_encodinginfo_t encoding;
        double volume;
        double replay_gain;
        sox_oob_t oob;
        sox_bool no_glob;
        sox_format_t* ft; /* libSoX file descriptor */
        uint64_t volume_clips;
        rg_mode replay_gain_mode;
    } file_t;
    file_t opts;
    inline char const* try_device(char const* name) {
        sox_format_handler_t const* handler = sox_find_format(name, sox_false);
        if (handler) {
            sox_format_t format, *ft = &format;
            lsx_debug("Looking for a default device: trying format `%s'", name);
            memset(ft, 0, sizeof(*ft));
            ft->filename = (char*)device_name(name);
            ft->priv = lsx_calloc(1, handler->priv_size);
            if (handler->startwrite(ft) == SOX_SUCCESS) {
                handler->stopwrite(ft);
                free(ft->priv);
                return name;
            }
            free(ft->priv);
        }
        return NULL;
    }
    inline char const* device_name(char const* const type) {
        const char *name = NULL, *from_env = getenv("AUDIODEV");

        if (!type)
            return NULL;

        if (0 || !strcmp(type, "sunau") || !strcmp(type, "oss") || !strcmp(type, "ossdsp") || !strcmp(type, "alsa") || !strcmp(type, "ao") || !strcmp(type, "sndio") || !strcmp(type, "coreaudio") || !strcmp(type, "pulseaudio") || !strcmp(type, "waveaudio"))
            name = "default";

        return name ? from_env ? from_env : name : NULL;
    }
    inline void init_file(file_t* f) {
        memset(f, 0, sizeof(*f));
        sox_init_encodinginfo(&f->encoding);
        f->volume = HUGE_VAL;
        f->replay_gain = HUGE_VAL;
    }
    inline char const* set_default_device(file_t* f) {
        /* Default audio driver type in order of preference: */
        if (!f->filetype)
            f->filetype = getenv("AUDIODRIVER");
        if (!f->filetype)
            f->filetype = try_device("coreaudio");
        if (!f->filetype)
            f->filetype = try_device("pulseaudio");
        if (!f->filetype)
            f->filetype = try_device("alsa");
        if (!f->filetype)
            f->filetype = try_device("waveaudio");
        if (!f->filetype)
            f->filetype = try_device("sndio");
        if (!f->filetype)
            f->filetype = try_device("oss");
        if (!f->filetype)
            f->filetype = try_device("sunau");

        if (!f->filetype) {
            lsx_fail("Sorry, there is no default audio device configured");
            exit(1);
        }
        return device_name(f->filetype);
    }

    inline streamPlayer() {
        assert(sox_init() == SOX_SUCCESS);
        createPlayer();
        printf("audio init\n");
    }
    inline void createPlayer() {
        if (out) {
            sox_close(out);
        }
        opts.filename = (char*)set_default_device(&opts);
        opts.signal.channels = 2;
        opts.signal.length = 0;
        opts.signal.mult = 0;
        opts.signal.precision = 16;
        opts.signal.rate = sampleRate;
        sox_init_encodinginfo(&opts.encoding);
        printf("createPlayer\n");
        printf("filename:%s\n", opts.filename);
        printf("filetype:%s\n", opts.filetype);
        out = sox_open_write(opts.filename, &opts.signal, &opts.encoding,
                             opts.filetype, NULL, NULL);
    }
    inline void setSampleRate(int r) {
        if (sampleRate != r) {
            sampleRate = r;
            createPlayer();
        }
    }
    inline void write(const sox_sample_t* buf) {
        sox_write(out, buf, 1024);
    }
    inline ~streamPlayer() {
        sox_close(out);
        sox_quit();
    }
};

}  // namespace mgnr::synthesizer
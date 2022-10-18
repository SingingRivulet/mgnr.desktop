#include "sf2.h"

namespace mgnr {
extern const char* instrumentName[128];

namespace synthesizer::vinstrument {

sf2::sf2(const char* path) {
    for (int i = 0; i < 128; ++i) {
        instrument2Id[instrumentName[i]] = i;
    }
    settings = new_fluid_settings();
    if (settings == NULL) {
        puts("Failed to create the settings!");
        exit(1);
    }

    synth = new_fluid_synth(settings);
    if (synth == NULL) {
        puts("Failed to create the synth!");
        exit(1);
    }

    auto sfont_id = fluid_synth_sfload(synth, path, 1);
    if (sfont_id == FLUID_FAILED) {
        puts("Loading the SoundFont failed!");
    }
}

void sf2::render(dataBlock* buffer) {
    float left[512], right[512];
    float *dry[1 * 2], *fx[1 * 2];
    dry[0] = left;
    dry[1] = right;
    fx[0] = left;
    fx[1] = right;
    memset(left, 0, sizeof(left));
    memset(right, 0, sizeof(right));
    int err = fluid_synth_process(synth, 512, 2, fx, 2, dry);
    if (err == FLUID_FAILED) {
        puts("oops\n");
    } else {
        for (int i = 0; i < 512; ++i) {
            buffer->buffer_channel[0][i] = left[i];
            buffer->buffer_channel[1][i] = right[i];
        }
    }
}

bool sf2::play_noteOn(note* n) {
    int ins = useInstrument(n->info);
    if (ins >= 0) {
        fluid_synth_noteon(synth, ins, n->tone, n->volume);
    } else {
        printf("mgenner:fail to play note:%s(%d)\n", n->info.c_str(), (int)n->tone);
    }
    return true;
}

bool sf2::play_noteOff(note* n) {
    int ins = releaseInstrument(n->info);
    if (ins >= 0) {
        fluid_synth_noteoff(synth, ins, n->tone);
    } else {
        printf("mgenner:fail to release note:%s(%d)\n", n->info.c_str(), (int)n->tone);
    }
    return true;
}

void sf2::play_stopAll() {
    for (int i = 0; i < 16; ++i) {
        playNum[i] = 0;
        playIns[i] = -1;
    }
    for (int i = 0; i < 128; ++i) {
        ins2Channel[i] = -1;
    }
}

sf2::~sf2() {
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

int sf2::useInstrument(const stringPool::stringPtr& n) {
    int id = getInstrumentId(n);
    if (ins2Channel[id] == -1) {
        for (int i = 0; i < 16; ++i) {
            if (playIns[i] == -1) {
                ins2Channel[id] = i;
                playIns[i] = id;
                playNum[i] = 1;
                fluid_synth_program_change(synth, i, id);
                return i;
            }
        }
        return -1;
    } else {
        ++playNum[ins2Channel[id]];
        return ins2Channel[id];
    }
}
int sf2::getInstrumentId(const stringPool::stringPtr& name) {
    char n[128];
    snprintf(n, 128, "%s", name.c_str());
    for (int i = 0; i < 128; ++i) {
        if (n[i] == '\0') {
            break;
        } else if (n[i] == '.') {
            n[i] = '\0';
            break;
        }
    }
    auto it = instrument2Id.find(n);
    if (it == instrument2Id.end()) {
        return 0;
    } else {
        return it->second;
    }
}
int sf2::releaseInstrument(const stringPool::stringPtr& n) {
    int id = getInstrumentId(n);
    if (ins2Channel[id] == -1) {
        return -1;
    } else {
        int c = ins2Channel[id];
        --playNum[c];
        if (playNum[c] <= 0) {
            playNum[c] = 0;
            playIns[c] = -1;
            ins2Channel[id] = -1;
        }
        return c;
    }
    return -1;
}
}  // namespace synthesizer::vinstrument
}  // namespace mgnr
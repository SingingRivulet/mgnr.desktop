#include "mgenner.h"
void mgenner::synth_init() {
    settings = new_fluid_settings();
    if (settings == NULL) {
        puts("Failed to create the settings!");
        exit(1);
    }

    /* Change the settings if necessary*/

    /* Create the synthesizer. */
    synth = new_fluid_synth(settings);
    if (synth == NULL) {
        puts("Failed to create the synth!");
        exit(1);
    }

    /* Load a SoundFont and reset presets (so that new instruments
     * get used from the SoundFont)
     * Depending on the size of the SoundFont, this will take some time to complete...
     */
    sfont_id = fluid_synth_sfload(synth, "../datas/soundfont/sndfnt.sf2", 1);
    if (sfont_id == FLUID_FAILED) {
        puts("Loading the SoundFont failed!");
        exit(1);
    }

    /* Create the audio driver. The synthesizer starts playing as soon
       as the driver is created. */
    adriver = new_fluid_audio_driver(settings, synth);
    if (adriver == NULL) {
        puts("Failed to create the audio driver!");
        exit(1);
    }
}

void mgenner::synth_shutdown() {
    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

void mgenner::onSetChannelIns(int c, int ins) {
    fluid_synth_program_change(synth, c, ins);
}

void mgenner::callSynthNoteOn(const char* info, int channel, int tone, int vol) {
    fluid_synth_noteon(synth, channel, tone, vol);
}

void mgenner::callSynthNoteOff(const char* info, int channel, int tone) {
    fluid_synth_noteoff(synth, channel, tone);
}
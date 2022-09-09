#include "editWindow.h"
void renderContext::synth_init() {
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
    sfont_id = fluid_synth_sfload(synth, path_sf2.c_str(), 1);
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

void renderContext::synth_shutdown() {
    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

void editWindow::onSetChannelIns(int c, int ins) {
    fluid_synth_program_change(parent->synth, c, ins);
}

void editWindow::callSynthNoteOn(const char* info, int channel, int tone, int vol) {
    fluid_synth_noteon(parent->synth, channel, tone, vol);
}

void editWindow::callSynthNoteOff(const char* info, int channel, int tone) {
    fluid_synth_noteoff(parent->synth, channel, tone);
}
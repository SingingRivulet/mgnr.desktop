#include "editWindow.h"
#include "synth/simple.h"
void renderContext::exportWav(const std::string& path) {
    mgnr::offlineSynth::simple::render(drawing, path_sf2.c_str(), 44100, path.c_str());
}
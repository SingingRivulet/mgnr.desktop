#include "mgenner.h"

int main(int argn, const char** argv) {
    mgenner m;
    if (argn >= 2) {
        m.openMidiFile(argv[1]);
    } else {
        m.createWindow();
    }
    while (m.running) {
        m.loop();
    }
    return 0;
}
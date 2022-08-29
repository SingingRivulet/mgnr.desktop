#include "mgenner.h"

int main(int argn, const char** argv) {
    mgenner m;
    if (argn >= 2) {
        m.loadMidiFile(argv[1]);
    }
    while (m.running) {
        m.loop();
    }
    return 0;
}
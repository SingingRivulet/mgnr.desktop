#include "mgenner.h"

int main() {
    mgenner m;
    while (m.running) {
        m.loop();
    }
    return 0;
}
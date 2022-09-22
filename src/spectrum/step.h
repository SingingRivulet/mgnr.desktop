#pragma once
#include "full.h"
namespace mgnr::spectrum {

struct stepRenderer {
    fullRenderer* parent;
    int len;
    std::vector<float> points;
    inline void get(int step) {
        points.clear();
        if (parent && step >= 0 && step < parent->width) {
            auto delta = parent->maxElement - parent->minElement;
            if (delta == 0) {
                delta = 1;
            }
            auto block = parent->data[step];
            len = parent->height;
            for (int i = 0; i < parent->height; ++i) {
                auto value = block[i];

                auto h = 1. - ((value - parent->minElement) / delta);

                points.push_back(h);
            }
        }
    }
};

}  // namespace mgnr::spectrum
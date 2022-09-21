#pragma once
#include "full.h"
namespace mgnr::spectrum {

struct stepRenderer {
    fullRenderer* parent;
    std::vector<std::pair<float, float>> points;
    inline void get(int step) {
        points.clear();
        if (parent && step >= 0 && step < parent->width) {
            auto delta = parent->maxElement - parent->minElement;
            if (delta == 0) {
                delta = 1;
            }
            for (int i = 0; i < parent->height; ++i) {
                auto value = parent->data[step * parent->height + i];

                auto h = 1. - ((value - parent->minElement) / delta);

                points.push_back(
                    std::make_pair(
                        ((float)i) / parent->height, h));
            }
        }
    }
};

}  // namespace mgnr::spectrum
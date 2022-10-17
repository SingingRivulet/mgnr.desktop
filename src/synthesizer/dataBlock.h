#pragma once
#include <memory>
#include <mutex>
#include "editTable.h"
//合成器
namespace mgnr::synthesizer {

struct dataBlock {
    typedef float buffer_channel[512] ;
    float buffer_channel[2];
};

}  // namespace mgnr::synthesizer
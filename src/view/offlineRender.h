#pragma once
#include <string>
#include "editWindow.h"
class offlineRender {
   public:
    bool done = false;
    bool close = false;
    float progress = 0;
    std::string path;
    bool render();
    virtual void stop() = 0;
    virtual void loop() = 0;
    virtual ~offlineRender();
};
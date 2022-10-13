#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include "WavFile.h"
#include "editWindow.h"
#include "offline.h"
#include "synth/simple.h"

bool offlineRender::render() {
    char buf[512];
    snprintf(buf, sizeof(buf), "导出wav##%s", path.c_str());
    if (ImGui::Begin(buf, nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("输出文件：%s", path.c_str());
        ImGui::ProgressBar(progress);
        if (done) {
            progress = 1;
            if (ImGui::Button("关闭")) {
                close = true;
            }
        } else {
            if (ImGui::Button("停止")) {
                stop();
            }
        }
    }
    bool focus = (ImGui::IsItemFocused() ||
                  ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
                  ImGui::IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows));
    ImGui::End();
    return focus;
}

class simpleRender : public offlineRender {
   public:
    mgnr::offline renderer;
    std::thread processThread;
    std::atomic<bool> processing = false;
    FILE* fp;
    simpleRender(editWindow* midi,
                 const char* sf,
                 int sampleRate,
                 const char* outpath)
        : renderer(sf, sampleRate) {
        for (auto it : midi->notes) {
            renderer.addNote(it->begin, it->tone, it->duration, it->volume, it->info);
        }
        renderer.timeMap = midi->timeMap;
        renderer.TPQ = midi->TPQ;
        renderer.updateTimeMax();
        float buf[64];
        path = outpath;
        fp = fopen(outpath, "w");
        if (fp) {
            std::thread p([&]() {
                processing = true;
                float buf[64];
                WavOutFile out(fp, sampleRate, 32, 1);
                while (processing && renderer.renderStep(buf)) {
                    out.write(buf, 64);
                    progress = (float)renderer.lookAtX / renderer.noteTimeMax;
                }
                processing = false;
                done = true;
            });
            processThread = std::move(p);
        }
    }
    ~simpleRender() {
        stop();
        if (fp) {
            fclose(fp);
        }
    }
    void stop() override {
        if (processing) {
            processing = false;
            processThread.join();
        }
    }
    void loop() override {
    }
};
void editWindow::exportWav(const std::string& path_sf2, const std::string& path) {
    auto p = std::make_unique<simpleRender>(this, path_sf2.c_str(), 44100, path.c_str());
    offlineRenderers.push_back(std::move(p));
}

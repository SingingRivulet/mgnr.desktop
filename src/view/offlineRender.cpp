#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include "WavFile.h"
#include "editWindow.h"
#include "offline.h"

#ifdef _WIN32
#include <Windows.h>
#endif

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
offlineRender::~offlineRender() {}

class simpleRender : public offlineRender {
   public:
    mgnr::offline renderer;
    std::thread processThread;
    std::atomic<bool> processing = false;
    FILE* fp;
    std::string tmpPath;
    int sampleRate;
    simpleRender(editWindow* midi,
                 const char* sf,
                 int sampleRate,
                 const char* outpath) {
        std::string tmpPath_dir = "/tmp";
#ifdef _WIN32
        TCHAR temp_file[255];
        GetTempPath(255, temp_file);
        tmpPath_dir = temp_file;
#endif
        this->sampleRate = sampleRate;
        tmpPath = tmpPath_dir + "/mgnrtmp" + std::to_string(rand()) + ".mid";
        midi->exportMidi(tmpPath, false);
        renderer.loadMidi(tmpPath);
        renderer.updateTimeMax();
        path = outpath;
        fp = fopen(outpath, "w");
        if (fp) {
            processThread = std::thread([this]() {
                processing = true;
                float buf[1024];
                WavOutFile out(fp, this->sampleRate, 32, 2);
                while (processing && renderer.renderStep(buf)) {
                    out.write(buf, 1024);
                    progress = (float)renderer.lookAtX / renderer.noteTimeMax;
                }
                processing = false;
                done = true;
            });
        }
    }
    ~simpleRender() override {
        stop();
        processThread.join();
        remove(tmpPath.c_str());
        printf("remove:%s\n", tmpPath.c_str());
    }
    void stop() override {
        processing = false;
    }
    void loop() override {
    }
};
void editWindow::exportWav(const std::string& path_sf2, const std::string& path) {
    auto p = std::make_unique<simpleRender>(this, path_sf2.c_str(), 44100, path.c_str());
    offlineRenderers.push_back(std::move(p));
}

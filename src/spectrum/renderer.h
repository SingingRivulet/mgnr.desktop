#pragma once

#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include <cmath>
#include <functional>
#include <list>
#include <map>
#include <memory>

#define chunkSize 128

namespace mgnr::spectrum {

template <typename T>
struct vec2 {
    T x, y;
    inline vec2() {
        x = 0;
        y = 0;
    }
    inline vec2(const vec2& i) {
        x = i.x;
        y = i.y;
    }
    inline vec2(T ix, T iy) {
        x = ix;
        y = iy;
    }
    inline vec2& operator=(const vec2& i) {
        x = i.x;
        y = i.y;
        return *this;
    }
};

struct viewPort_t {
    vec2<int> lookAtBegin, lookAtEnd, windowSize;
    float scale = 1;  //参数越大图片越小，看的区域越大

    inline viewPort_t() {}

    inline void setLookAt(const vec2<int>& pos) {
        lookAtBegin.x = pos.x;
        lookAtBegin.y = pos.y;
        updateLookAt();
    }
    inline void updateLookAt() {
        lookAtEnd.x = lookAtBegin.x + windowSize.x * scale;
        lookAtEnd.y = lookAtBegin.y + windowSize.y * scale;
    }
};

struct chunk_t {
    vec2<int> pos;
    vec2<int> drawPosMin, drawPosMax;
    vec2<float> srcPosMin, srcPosMax;
    bool visible;
    int updateFlag = 0;
    SDL_Texture* texture;
    inline chunk_t(SDL_Surface* surface, SDL_Renderer* renderer) {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
    }
    inline ~chunk_t() {
        SDL_DestroyTexture(texture);
    }
};

struct LOD {
    viewPort_t* viewPort;
    int scale = 1;  //参数越大图片越小，看的区域越大
    std::map<std::pair<int, int>, std::unique_ptr<chunk_t>> chunks;
    inline LOD() {}
    inline ~LOD() {
        //printf("spectrum:release lod=%d\n", scale);
    }
};

inline int getScale(double oscale) {
    auto res = std::floor(std::log2(oscale));
    if (res < 0) {
        res = 0;
    }
    return 1 << ((int)res);
}

inline bool getRenderPos(viewPort_t& viewPort,
                         LOD& lod,
                         chunk_t& chunk) {
    //真实位置  = chunk.pos * lod.scale * chunkSize
    //真实尺寸  = chunkSize * lod.scale
    //渲染位置  = (真实位置 - viewPort.lookAtBegin)/viewPort.scale
    //         = (chunk.pos * lod.scale * chunkSize - viewPort.lookAtBegin)/viewPort.scale
    //渲染尺寸  = 真实尺寸/viewPort.scale
    //         = chunkSize * lod.scale/viewPort.scale
    int d_beginX = (chunk.pos.x * lod.scale * chunkSize - viewPort.lookAtBegin.x) / viewPort.scale;
    int d_beginY = (chunk.pos.y * lod.scale * chunkSize - viewPort.lookAtBegin.y) / viewPort.scale;
    int d_sizeX = chunkSize * lod.scale / viewPort.scale;
    int d_sizeY = chunkSize * lod.scale / viewPort.scale;

    int d_endX = d_beginX + d_sizeX;
    int d_endY = d_beginY + d_sizeY;

    float s_beginX = 0;
    float s_beginY = 0;
    float s_endX = 1;
    float s_endY = 1;

    if (d_beginX >= viewPort.windowSize.x ||
        d_beginY >= viewPort.windowSize.y ||
        d_endX < 0 ||
        d_endY < 0) {
        return false;
    }
    //裁剪
    if (d_endX >= viewPort.windowSize.x) {
        int now_size = d_sizeX;
        int target_size = viewPort.windowSize.x - d_beginX - 1;
        double delta = ((double)target_size) / ((double)now_size);
        d_endX = viewPort.windowSize.x - 1;
        s_endX = delta;
    }
    if (d_endY >= viewPort.windowSize.y) {
        int now_size = d_sizeY;
        int target_size = viewPort.windowSize.y - d_beginY - 1;
        double delta = ((double)target_size) / ((double)now_size);
        d_endY = viewPort.windowSize.y - 1;
        s_endY = delta;
    }
    if (d_beginX < 0) {
        int now_size = d_sizeX;
        int target_size = -d_beginX;
        double delta = ((double)target_size) / ((double)now_size);
        d_beginX = 0;
        s_beginX = delta;
    }
    if (d_beginY < 0) {
        int now_size = d_sizeY;
        int target_size = -d_beginY;
        double delta = ((double)target_size) / ((double)now_size);
        d_beginY = 0;
        s_beginY = delta;
    }
    chunk.drawPosMin.x = d_beginX;
    chunk.drawPosMin.y = d_beginY;
    chunk.drawPosMax.x = d_endX;
    chunk.drawPosMax.y = d_endY;
    chunk.srcPosMin.x = s_beginX;
    chunk.srcPosMin.y = s_beginY;
    chunk.srcPosMax.x = s_endX;
    chunk.srcPosMax.y = s_endY;
    return true;
}

constexpr Uint32 Amask = 0xFF000000;
constexpr Uint32 Rmask = 0x00FF0000;
constexpr Uint32 Gmask = 0x0000FF00;
constexpr Uint32 Bmask = 0x000000FF;

struct renderer {
    std::shared_ptr<LOD> layout_show;
    std::shared_ptr<LOD> layout_draw;
    viewPort_t viewPort;
    int updateFlag = 0;
    SDL_Renderer* sdlrenderer;

    float maxElement;
    std::vector<float> data;
    int width = 0;
    int height = 0;

    bool needUpdate = true;

    inline renderer() {
        viewPort.scale = 1;
        //printf("spectrum:viewPort.scale=%f\n", viewPort.scale);
    }
    inline void updateDate() {
        maxElement = 1;
        for (auto& it : data) {
            if (it > maxElement) {
                maxElement = it;
            }
        }
    }
    inline void update() {
        //图片变化的时候才需要调用
        int s = getScale(viewPort.scale);
        if (layout_draw != nullptr) {
            if (layout_draw->scale != s) {
                layout_draw = std::shared_ptr<LOD>(new LOD);
                layout_draw->scale = s;
                //printf("spectrum:layout_draw != nullptr\n");
            }
        } else {
            if (layout_show != nullptr && s == layout_show->scale) {
                layout_draw = layout_show;
                //printf("spectrum:layout_show != nullptr && s == layout_show->scale\n");
            } else {
                layout_draw = std::shared_ptr<LOD>(new LOD);
                layout_draw->scale = s;
                //printf("spectrum:create\n");
            }
        }
        //printf("spectrum:layout_draw->scale=%d viewPort.scale=%f\n",
        //       layout_draw->scale,
        //       viewPort.scale);
        viewPort.updateLookAt();
        layout_draw->viewPort = &viewPort;
        updateChunks();
        if (layout_show != nullptr) {
            for (auto& it : layout_show->chunks) {
                it.second->visible = getRenderPos(viewPort, *layout_show, *it.second);
            }
        }
    }
    inline void render() {
        if (needUpdate) {
            update();
            needUpdate = false;
        }
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        //printf("spectrum:CursorScreenPos=(%f,%f)\n", p0.x, p0.y);
        if (layout_show != nullptr) {
            for (auto& it : layout_show->chunks) {
                if (it.second->visible) {
                    //绘制chunk
                    //printf("spectrum:pos=(%d,%d) drawPos=(%d,%d) drawSize=(%d,%d)\n",
                    //       it.second->pos.x, it.second->pos.y,
                    //       it.second->drawPos.x, it.second->drawPos.y,
                    //       it.second->drawSize.x, it.second->drawSize.y);
                    draw_list->AddImage(
                        (ImTextureID)it.second->texture,
                        ImVec2(it.second->drawPosMin.x + p0.x, it.second->drawPosMin.y + p0.y),
                        ImVec2(it.second->drawPosMax.x + p0.x, it.second->drawPosMax.y + p0.y),
                        ImVec2(it.second->srcPosMin.x, it.second->srcPosMin.y),
                        ImVec2(it.second->srcPosMax.x, it.second->srcPosMax.y));
                }
            }
        }
    }
    inline void updateChunks() {
        if (layout_draw != nullptr) {
            ++updateFlag;
            viewPort.updateLookAt();
            int fx = std::floor((viewPort.lookAtBegin.x * viewPort.scale) / (layout_draw->scale * chunkSize));
            int fy = std::floor((viewPort.lookAtBegin.y * viewPort.scale) / (layout_draw->scale * chunkSize));
            int ex = std::ceil((viewPort.lookAtEnd.x * viewPort.scale) / (layout_draw->scale * chunkSize));
            int ey = std::ceil((viewPort.lookAtEnd.y * viewPort.scale) / (layout_draw->scale * chunkSize));
            //printf("spectrum:layout_draw->scale=%d viewPort.scale=%f\n", layout_draw->scale, viewPort.scale);
            //printf("spectrum:viewPort.lookAtBegin=(%d,%d)\n",
            //       viewPort.lookAtBegin.x,
            //       viewPort.lookAtBegin.y);
            //printf("spectrum:viewPort.lookAtEnd=(%d,%d)\n",
            //       viewPort.lookAtEnd.x,
            //       viewPort.lookAtEnd.y);
            //printf("spectrum:fx=%d fy=%d ex=%d ey=%d\n", fx, fy, ex, ey);
            for (int x = fx; x <= ex; ++x) {
                for (int y = fy; y <= ey; ++y) {
                    auto key = std::make_pair(x, y);
                    auto chunkIt = layout_draw->chunks.find(key);
                    if (chunkIt == layout_draw->chunks.end()) {
                        genChunk(x, y);  //创建区块
                    } else {
                        chunkIt->second->updateFlag = updateFlag;
                    }
                }
            }
            //删除看不见的chunk
            std::erase_if(layout_draw->chunks, [&](const auto& p) {
                return p.second->updateFlag != updateFlag;
            });
            //printf("spectrum:chunk num=%d\n", layout_draw->chunks.size());
            layout_show = layout_draw;
            layout_draw = nullptr;
        }
    }
    inline void genChunk(int x, int y) {
        //printf("spectrum:genChunk:%d %d\n", x, y);
        int fx = x * layout_draw->scale * chunkSize;
        int fy = y * layout_draw->scale * chunkSize;
        int ex = (x + 1) * layout_draw->scale * chunkSize;
        int ey = (y + 1) * layout_draw->scale * chunkSize;
        //检查是否在区域内
        if (ex < 0 || ey < 0 || fx > width || fy > height) {
            return;
        }
        int delta = layout_draw->scale;

        auto surface = SDL_CreateRGBSurface(0, chunkSize, chunkSize, 32,
                                            Rmask, Gmask,
                                            Bmask, Amask);
        if (SDL_MUSTLOCK(surface)) {
            SDL_LockSurface(surface);
        }
        auto data = (Uint8*)surface->pixels;

        for (int i = 0; i < chunkSize; ++i) {
            int px = fx + i * delta;
            for (int j = 0; j < chunkSize; ++j) {
                int py = fy + j * delta;
                //像素点的坐标为(x,y)
                data[(j * chunkSize + i) * 4] = getPixel(px, py);
                data[(j * chunkSize + i) * 4 + 1] = getPixel(px, py);
                data[(j * chunkSize + i) * 4 + 2] = getPixel(px, py);
                data[(j * chunkSize + i) * 4 + 3] = getPixel(px, py);
            }
        }

        if (SDL_MUSTLOCK(surface)) {
            SDL_UnlockSurface(surface);
        }
        auto ptr = std::make_unique<chunk_t>(surface, sdlrenderer);
        ptr->updateFlag = updateFlag;
        ptr->pos.x = x;
        ptr->pos.y = y;
        layout_draw->chunks[std::make_pair(x, y)] = std::move(ptr);
        SDL_FreeSurface(surface);
    }
    inline Uint8 getPixel(int x, int y) {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            //printf("spectrum:getPixel(%d,%d)\n",x,y);
            return 0;
        }
        auto res = (data[x * height + (height - y - 1)] / maxElement) * 255;
        res = std::max(res, 0.f);
        res = std::min(res, 255.f);
        //printf("spectrum:getPixel(%d,%d)=%d\n", x, y, int(res));
        return res;
    }
};

}  // namespace mgnr::spectrum

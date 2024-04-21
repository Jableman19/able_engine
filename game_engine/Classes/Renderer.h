// Renderer.h
#pragma once

#include "SDL2/SDL.h"
#include <vector>
#include <string>
#include <queue>
#include <glm/vec2.hpp>
#include "ImageDrawRequest.h"
#include "Text.h"
#include "EngineUtils.h"

namespace Renderer {
    // Variable declarations
    extern int clear_color_r;
    extern int clear_color_g;
    extern int clear_color_b;
    extern glm::vec2 camPos;

    extern std::queue<ImageDrawRequest> renderQueue;

    extern float camera_offset_x;
    extern float camera_offset_y;

    extern float zoom_factor;

    extern SDL_Renderer* sdlRenderer;

    extern int window_width;
    extern int window_height;
    extern int phase;

    // Function declarations
    void Render();
    void renderText(std::string text, int x, int y);
    void renderText(Text* text);
    void renderImage(SDL_Texture* texture, int x, int y, int w, int h);
    void renderImage(SDL_Texture* texture, int x, int y);
    void renderImage(SDL_Texture* texture);
    void SetPosition(float x, float y);
    float GetPositionX();
    float GetPositionY();
    void SetZoom(float zoom);
    float GetZoom();
    int getEvents();
    void clear();
    void start(SDL_Window** window, EngineUtils& engineUtils, rapidjson::Document& jsonDoc, bool rendering);

    // ... Add any other declarations for functions or variables that are part of the Renderer namespace ...
};

#pragma once
#include "SDL2_image/SDL_image.h"
#include <unordered_map>
#include <string>
#include <iostream>
#include <queue>
#include <algorithm>
#include "ImageDrawRequest.h"
#include "glm/glm.hpp"	
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "EngineUtils.h"
namespace ImageDB {

		extern SDL_Renderer* renderer;

		extern std::unordered_map<std::string, SDL_Texture*> imageCache;

		extern std::vector<ImageDrawRequest> renderQueue;

		void addImages(std::vector<std::string> names);

		void addTextImage(std::string name, SDL_Texture* texture);

		void addImage(std::string name);

		void RenderAndClearAllImages(float camera_zoom, glm::vec2 cameraPos, glm::ivec2 camDim, int clearR, int clearG, int clearB);

		void DrawUI(std::string image_name, float x, float y);
		void DrawUIEx(std::string image_name, float x, float y, float r, float g, float b, float a, float sorting_order);
		void Draw(std::string image_name, float x, float y);
		void DrawEx(std::string image_name, float x, float y,float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order);
		void DrawPixel(float x, float y, float r, float g, float b, float a);
		void SetColorMode(std::string mode);
		void SetColorModeI(int mode);

		SDL_Texture* GetImage(std::string name);

};
#pragma once
#ifndef TEXT_DB_H
#define TEXT_DB_H
#include <unordered_map>
#include <SDL2_ttf/SDL_ttf.h>
#include <queue>
#include "Text.h"
#include "ImageDrawRequest.h"

namespace TextDB {
	extern std::unordered_map<std::string, TTF_Font*> fonts;
	extern SDL_Color fontColor;
	extern std::queue<Text*> renderQueue;
	extern SDL_Renderer* sdlRenderer;
	extern int windowWidth;
	extern int windowHeight;

	SDL_Surface* RenderText(Text* text);
	void Draw(std::string text, float x, float y, std::string font_name, int font_size, int r, int g, int b, int a);
	void DrawSubtitle(std::string text);
	void LoadFont(const std::string& fontName, int font_size);
	void LoadBuiltInFonts();
};
#endif

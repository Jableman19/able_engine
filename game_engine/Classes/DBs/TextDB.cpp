#include "TextDB.h"
#include "ImageDB.h"
#include <iostream>

namespace TextDB {
	//font map
	std::unordered_map<std::string, TTF_Font*> fonts;
	SDL_Color fontColor = { 0, 0, 0, 255 }; // Initial value, if needed.
	std::queue<Text*> renderQueue;
	//define renderer
	SDL_Renderer* sdlRenderer;
	int windowWidth;
	int windowHeight;


	SDL_Surface* RenderText(Text* text) {
		fontColor.r = 255;
		fontColor.g = 255;
		fontColor.b = 255;
		//fontColor.a = text->a;
		return TTF_RenderText_Solid(fonts[text->font_name], text->text.c_str(), fontColor);
	}

	void Draw(std::string text, float x, float y, std::string font_name, int font_size, int r, int g, int b, int a) {
		// Implementation...
		Text* textObj = new Text(text, x, y, font_name, font_size, r, g, b, a);
		LoadFont(font_name, font_size);
		SDL_Surface* surface = RenderText(textObj);
		SDL_Texture* SDLTXT = SDL_CreateTextureFromSurface(sdlRenderer, surface);
		ImageDB::addTextImage(text, SDLTXT);
		ImageDrawRequest textReq = { text, x, y, 0, 1.0f, 1.0f, 0.0f, 0.0f, r,g,b,a, 1 };
		//add to render queue
		ImageDB::renderQueue.push_back(textReq);
	}

	void DrawSubtitle(std::string text){
		Text* textObj = new Text(text, windowWidth / 2, windowHeight * .75, "NotoSans-Regular", 24, 255, 255, 0, 255);
		SDL_Surface* surface = RenderText(textObj);
		SDL_Texture* SDLTXT = SDL_CreateTextureFromSurface(sdlRenderer, surface);
		ImageDB::addTextImage(text, SDLTXT);
		ImageDrawRequest textReq = { text, windowWidth / 2, windowHeight * .75, 0, 1.0f, 1.0f, 0.0f, 0.0f, 255,255,0,255, 1 };
		//add to render queue
		ImageDB::renderQueue.push_back(textReq);
	}

	void LoadBuiltInFonts() {
		fonts["NotoSans-Regular"] = TTF_OpenFont("builtin/NotoSans-Regular.ttf", 24);
	}

	void LoadFont(const std::string& fontName, int font_size) {
		fonts[fontName] = TTF_OpenFont(("resources/fonts/" + fontName + ".ttf").c_str(), font_size);
		if (fonts[fontName] == nullptr) {
			std::cout << "error: font " + fontName + " missing\n";
			exit(0);
		}
	}
}

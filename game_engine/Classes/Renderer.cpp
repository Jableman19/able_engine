#include "SDL2/SDL.h"
#include <vector>
#include <string>
#include <queue>
#include "DBs/ImageDB.h"
#include "DBs/TextDB.h"
#include "DBs/AudioDB.h"
#include "Helper.h"
#include "AudioHelper.h"
#include "EngineUtils.h"
#include "Input.h"
#include "ImageDrawRequest.h"

namespace Renderer {
	int clear_color_r = 0;
	int clear_color_g = 0;
	int clear_color_b = 0;
	glm::vec2 camPos = glm::vec2(0.0f, 0.0f);

	std::queue<ImageDrawRequest> renderQueue;


	float camera_offset_x = 0.0f;
	float camera_offset_y = 0.0f;

	float zoom_factor = 1.0f;

	SDL_Renderer* sdlRenderer = nullptr;
	int window_width = 640;
	int window_height = 360;
	int phase = 0; //0 = intro, 1 = gameplay, 2 = game over good, 3 = game over bad
	//Player* player;

	void Render() {
		SDL_SetRenderDrawColor(sdlRenderer, clear_color_r, clear_color_g, clear_color_b, 255);
		ImageDB::RenderAndClearAllImages(zoom_factor, camPos, glm::ivec2{ window_width, window_height }, clear_color_r, clear_color_g, clear_color_b);
		Helper::SDL_RenderPresent498(sdlRenderer);
	}

	void renderImage(SDL_Texture* texture, int x, int y, int w, int h) {
		SDL_Rect dest;
		dest.x = x;
		dest.y = y;
		dest.w = w;
		dest.h = h;
		SDL_RenderCopy(sdlRenderer, texture, NULL, &dest);
	}

	//takes whole image width/height
	void renderImage(SDL_Texture* texture, int x, int y) {
		SDL_Rect dest;
		dest.x = x;
		dest.y = y;
		SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
		Helper::SDL_RenderCopyEx498(0, "", sdlRenderer, texture, NULL, &dest, 0, NULL, SDL_FLIP_NONE);
	}

	void renderImage(SDL_Texture* texture) {
		SDL_RenderCopy(sdlRenderer, texture, NULL, NULL);
	}

	void SetPosition(float x, float y) {
		camPos = glm::vec2(x, y);
	}

	glm::vec2 ScreenToWorld(float sX, float sY) {
	    //100 pixels = 1 unit, consider zoom
		return glm::vec2((sX - window_width / 2) / (100.0f * zoom_factor) + camPos.x, (sY - window_height / 2) / (100.0f * zoom_factor) + camPos.y);
	}

	float GetPositionX() {
		return camPos.x;
	}

	float GetPositionY() {
		return camPos.y;
	}

	void SetZoom(float zoom) {
		zoom_factor = zoom;
	}

	float GetZoom() {
		return zoom_factor;
	}
	int getEvents() {
		SDL_Event event;
		int retVal = 0;
		while (Helper::SDL_PollEvent498(&event)) {
			Input::ProcessEvent(event);
			//std::cout << "event type: " << event.type << std::endl;
			if (event.type == SDL_QUIT) {
				retVal = 1;
			}
		}
		return retVal;
	}

	void clear() {
		SDL_RenderClear(sdlRenderer);
		SDL_SetRenderDrawColor(sdlRenderer, clear_color_r, clear_color_g, clear_color_b, 255);
	}

	void start(SDL_Window** window, int r, int g, int b) {
		clear_color_b = b;
		clear_color_g = g;
		clear_color_r = r;

		sdlRenderer = Helper::SDL_CreateRenderer498(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
		SDL_RenderClear(sdlRenderer);
		//start DBs
		TextDB::sdlRenderer = sdlRenderer;
		ImageDB::renderer = sdlRenderer;
	}
};

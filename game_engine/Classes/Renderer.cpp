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

	//std::vector<std::string> messagesToShow;
	//std::vector<std::string> intro_images;
	//std::vector<std::string> intro_text;

	//std::string game_over_bad_image;
	//std::string game_over_good_image;
	//int intro_images_index;
	//int intro_text_index;
	int window_width = 640;
	int window_height = 360;
	int phase = 0; //0 = intro, 1 = gameplay, 2 = game over good, 3 = game over bad
	//Player* player;

	void Render() {
		SDL_SetRenderDrawColor(sdlRenderer, clear_color_r, clear_color_g, clear_color_b, 255);
		//render all images in queue
		/*while (!TextDB::renderQueue.empty()) {
			renderText(TextDB::renderQueue.front());
			TextDB::renderQueue.pop();
		}*/

		ImageDB::RenderAndClearAllImages(zoom_factor, camPos, glm::ivec2{ window_width, window_height }, clear_color_r, clear_color_g, clear_color_b);

		////render intro images
		//if(intro_images_index < intro_images.size() || intro_text_index < intro_text.size()) {
		//	//if image index bigger, set it to one less than the size
		//	if (intro_images_index >= intro_images.size()) {
		//		intro_images_index = intro_images.size() - 1;
		//	}
		//	//if text index bigger, set it to one less than the size
		//	if (intro_text_index >= intro_text.size()) {
		//		intro_text_index = intro_text.size() - 1;
		//	}
		//	//render image
		//	renderImage(imageDB->GetImage(intro_images[intro_images_index]));
		//	//render text
		//	if (intro_text_index >= 0) {
		//		renderIntroText(intro_text[intro_text_index]);
		//	}
		//}
		//else if(phase == 0){
		//	phase = 1;
		//	if(audioDB->intro_music != "") {
		//		audioDB->Halt(0);
		//	}
		//	if(audioDB->gameplay_music != "") {
		//		audioDB->Play(audioDB->gameplay_music, 0, -1);
		//	}
		//}
		//if(phase == 1) {
		//	//if there are no intro images, play gameplay music
		//	if (!intro_images.size() && intro_images_index == 0 && audioDB->gameplay_music != "") {
		//		audioDB->Play(audioDB->gameplay_music, 0, -1);
		//		intro_images_index++;
		//	}
		//	if (player->actor) {
		//		//HUD
		//		// render score
		//		renderText("score : " + std::to_string(player->score), 5, 5);

		//		//render image for each life left
		//		for (int i = 0; i < player->health; i++) {
		//			//The (x,y) position of image draw should be (5,25) + (i * (w + 5), 0)
		//			//Where w is the width in pixels of the hp_image and i is the icon index.
		//			//get width
		//			int w;
		//			SDL_QueryTexture(player->hp_texture, NULL, NULL, &w, NULL);
		//			//std::cout << "w: " << w << std::endl;
		//			renderImage(player->hp_texture, 5 + (i * ( w + 5)), 25);

		//		}

		//		//Messages
		//		for (int i = 0; i < messagesToShow.size(); i++) {
		//			renderText(messagesToShow[i], 25, window_height - 50 - (messagesToShow.size() - 1 - i) * 50);
		//		}
		//	}
		//}
		//else if(phase == 2 || phase == 4) {
		//	//if there is an image for game over good
		//	if (game_over_good_image != "") {
		//		//render game over good image
		//		renderImage(imageDB->GetImage(game_over_good_image));
		//	}
		//	else {
		//		exit(0);
		//	}
		//	//halt audio and play game over music
		//	if (phase == 2) {
		//		phase = 4;
		//		audioDB->Halt(0);
		//		audioDB->Play(audioDB->game_over_good_music, 0, 0);
		//	}
		//}
		//else if(phase == 3 || phase == 5) {
		//	if(game_over_bad_image != "") {
		//		//render game over bad image
		//		renderImage(imageDB->GetImage(game_over_bad_image));
		//	}
		//	else {
		//		exit(0);
		//	}
		//	//halt audio and play game over music
		//	if (phase == 3) {
		//		phase = 5;
		//		audioDB->Halt(0);
		//		audioDB->Play(audioDB->game_over_bad_music, 0, 0);
		//	}
		//}

		Helper::SDL_RenderPresent498(sdlRenderer);
	}

	void renderText(std::string text, int x, int y) {
		//renderImage(textDB->GetText(text, sdlRenderer), x, y);
	}

	void renderText(Text* text) {

		//renderImage(SDLTXT, text->x, text->y);
	}

	/*void renderIntroText(std::string text) {
		renderImage(textDB->GetText(text, sdlRenderer), 25, window_height - 50);
	}*/

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
		//SDL_RenderCopy(sdlRenderer, texture, NULL, &dest);
		Helper::SDL_RenderCopyEx498(0, "", sdlRenderer, texture, NULL, &dest, 0, NULL, SDL_FLIP_NONE);
	}

	void renderImage(SDL_Texture* texture) {
		SDL_RenderCopy(sdlRenderer, texture, NULL, NULL);
	}

	void SetPosition(float x, float y) {
		camPos = glm::vec2(x, y);
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
			//	if (phase == 0) {

			//		//get mouse click using scan code
			//		if (event.type == SDL_MOUSEBUTTONDOWN) {
			//			//left click
			//			if (event.button.button == SDL_BUTTON_LEFT) {
			//				//still have intro objects
			//				if (intro_images_index < intro_images.size()) {
			//					intro_images_index++;
			//				}
			//				//still have intro text
			//				if (intro_text_index < intro_text.size()) {
			//					intro_text_index++;
			//				}
			//			}
			//		
			//		}
			//		//get keyboard input
			//		if (event.type == SDL_KEYDOWN) {
			//			//printf("key pressed\n");
			//			//if enter key
			//			if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
			//				//printf("enter key pressed\n");
			//				//still have intro objects
			//				if (intro_images_index < intro_images.size()) {
			//					intro_images_index++;
			//				}
			//				//still have intro text
			//				if (intro_text_index < intro_text.size()) {
			//					intro_text_index++;
			//				}
			//			}
			//			//if space key
			//			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			//				//printf("space key pressed\n");
			//				//still have intro objects
			//				if (intro_images_index < intro_images.size()) {
			//					intro_images_index++;
			//				}
			//				//still have intro text
			//				if (intro_text_index < intro_text.size()) {
			//					intro_text_index++;
			//				}
			//			}
			//		}
			//	}
			//	/*else {
			//		
			//	}*/
			//}
			//if (player->actor != nullptr && phase != 0) {
			//	//get keyboard input
			//	glm::vec2 deltaVelocity = glm::vec2(0.0, 0.0);
			//	//if left key
			//	if (Input::GetKey(SDL_SCANCODE_LEFT) || Input::GetKey(SDL_SCANCODE_A)) {
			//		deltaVelocity.x -= player->speed;
			//	}
			//	if (Input::GetKey(SDL_SCANCODE_RIGHT) || Input::GetKey(SDL_SCANCODE_D)) {
			//		deltaVelocity.x += player->speed;
			//	}
			//	if (Input::GetKey(SDL_SCANCODE_UP) || Input::GetKey(SDL_SCANCODE_W)) {
			//		deltaVelocity.y -= player->speed;
			//	}
			//	if (Input::GetKey(SDL_SCANCODE_DOWN) || Input::GetKey(SDL_SCANCODE_S)) {
			//		deltaVelocity.y += player->speed;
			//	}
			//	//normalize movement, so that diagonal movement is not faster
			//	if (deltaVelocity.x != 0 && deltaVelocity.y != 0) {
			//		glm::vec2 norm = glm::normalize(deltaVelocity);
			//		deltaVelocity.x = norm.x * player->speed;
			//		deltaVelocity.y = norm.y * player->speed;
			//	}

			//	//update player position
			//	player->new_pos.x += deltaVelocity.x;
			//	player->new_pos.y += deltaVelocity.y;

			//	//change player actor vel based on deltaVelocity
			//	//player->actor->velocity = deltaVelocity;
		}
		return retVal;
	}

	void clear() {
		SDL_RenderClear(sdlRenderer);
		SDL_SetRenderDrawColor(sdlRenderer, clear_color_r, clear_color_g, clear_color_b, 255);
	}

	void start(SDL_Window** window, EngineUtils& engineUtils, rapidjson::Document& jsonDoc, bool rendering) {
		//check resources/rendering.config for colors
		if (rendering) {
			if (jsonDoc.HasMember("clear_color_r")) {
				clear_color_r = jsonDoc["clear_color_r"].GetInt();
			}
			if (jsonDoc.HasMember("clear_color_g")) {
				clear_color_g = jsonDoc["clear_color_g"].GetInt();
			}
			if (jsonDoc.HasMember("clear_color_b")) {
				clear_color_b = jsonDoc["clear_color_b"].GetInt();
			}
		}

		sdlRenderer = Helper::SDL_CreateRenderer498(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		SDL_SetRenderDrawColor(sdlRenderer, clear_color_r, clear_color_g, clear_color_b, 255);
		SDL_RenderClear(sdlRenderer);
		//start DBs
		TextDB::sdlRenderer = sdlRenderer;
		ImageDB::renderer = sdlRenderer;
		/*	textDB = new TextDB();*/
	}
};

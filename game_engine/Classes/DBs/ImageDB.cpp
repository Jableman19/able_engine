#include "ImageDB.h"

namespace ImageDB {
	SDL_Renderer* renderer;
	//image cache
	std::unordered_map<std::string, SDL_Texture*> imageCache;
	std::vector<ImageDrawRequest> renderQueue;
	int colorMode = 5;

	void adjustForDeuteranomaly(Uint8* r, Uint8* g, Uint8* b) {
		float newR = *r * 0.8 + *g * 0.2;
		float newG = *r * 0.2 + *g * 0.8;
		float newB = *b;

		*r = (Uint8)newR;
		*g = (Uint8)newG;
		*b = (Uint8)newB;
	}

	void adjustForProtanopia(Uint8* r, Uint8* g, Uint8* b) {
		float newR = *r * 0.567 + *g * 0.433;
		float newG = *r * 0.558 + *g * 0.442;
		float newB = *r * 0.242 + *g * 0.230 + *b * 0.528;

		*r = (Uint8)newR;
		*g = (Uint8)newG;
		*b = (Uint8)newB;
	}

	void adjustForDeuteranopia(Uint8* r, Uint8* g, Uint8* b) {
		float newR = *r * 0.625 + *g * 0.375;
		float newG = *r * 0.7 + *g * 0.3;
		float newB = *b; // Blue remains unchanged

		*r = (Uint8)newR;
		*g = (Uint8)newG;
		*b = (Uint8)newB;
	}


	void setColor(SDL_Renderer* renderer, Uint8& r, Uint8& g, Uint8& b, Uint8& a, int colorMode) {
		switch (colorMode) {
		case 0:
			adjustForDeuteranomaly(&r, &g, &b);
			break;
		case 1:
			adjustForProtanopia(&r, &g, &b);
			break;
		case 2:
			adjustForDeuteranopia(&r, &g, &b);
			break;
		default:
			break;
		}
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
	}


	SDL_Surface* adjustSurfaceForColorblindness(SDL_Surface* surface, int colorMode) {
		// Check if the surface must be locked
		if (SDL_MUSTLOCK(surface)) {
			SDL_LockSurface(surface);
		}

		int bpp = surface->format->BytesPerPixel; // Bytes per pixel
		Uint8* pixels = (Uint8*)surface->pixels;
		for (int y = 0; y < surface->h; y++) {
			for (int x = 0; x < surface->w; x++) {
				// Calculate the position of the pixel based on bpp
				Uint8* pixel_ptr = pixels + y * surface->pitch + x * bpp;

				// Extract color components
				Uint32 pixel = 0;
				memcpy(&pixel, pixel_ptr, bpp); // Copy the pixel data safely

				Uint8 r, g, b, a;
				SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);

				// Adjust color based on mode
				setColor(NULL, r, g, b, a, colorMode); // Modify r, g, b according to colorMode

				// Set pixel color back
				pixel = SDL_MapRGBA(surface->format, r, g, b, a);
				memcpy(pixel_ptr, &pixel, bpp); // Copy back modified pixel data safely
			}
		}

		// Unlock the surface if it was locked
		if (SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}

		return surface;
	}


	void DrawUI(std::string image_name, float x, float y){
		ImageDrawRequest request;
		request.image_name = image_name;
		request.x = x;
		request.y = y;
		request.r = 255;
		request.g = 255;
		request.b = 255;
		request.a = 255;
		request.sorting_order = 0;
		request.scene_space = false;
		renderQueue.push_back(request);
	}

	void DrawUIEx(std::string image_name, float x, float y, float r, float g, float b, float a, float sorting_order) {
		ImageDrawRequest request;
		request.image_name = image_name;
		request.x = x;
		request.y = y;
		request.r = static_cast<int>(r);
		request.g = static_cast<int>(g);
		request.b = static_cast<int>(b);
		request.a = static_cast<int>(a);
		request.sorting_order = static_cast<int>(sorting_order);
		request.scene_space = false;
		renderQueue.push_back(request);
	}

	void Draw(std::string image_name, float x, float y) {
		ImageDrawRequest request;
		request.image_name = image_name;
		request.x = x;
		request.y = y;
		request.r = 255;
		request.g = 255;
		request.b = 255;
		request.a = 255;
		request.pivot_x = 0.5f;
		request.pivot_y = 0.5f;
		request.sorting_order = -1;
		request.scene_space = true;
		renderQueue.push_back(request);
	}

	void DrawEx(std::string image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order){
		ImageDrawRequest request;
		request.image_name = image_name;
		request.x = x;
		request.y = y;
		request.rotation_degrees = static_cast<int>(rotation_degrees);
		request.scale_x = scale_x;
		request.scale_y = scale_y;
		request.pivot_x = pivot_x;
		request.pivot_y = pivot_y;
		request.r = static_cast<int>(r);
		request.g = static_cast<int>(g);
		request.b = static_cast<int>(b);
		request.a = static_cast<int>(a);
		request.sorting_order = static_cast<int>(sorting_order);
		request.scene_space = true;
		renderQueue.push_back(request);
	}

	void DrawPixel(float x, float y, float r, float g, float b, float a) {
		ImageDrawRequest request;
		request.image_name = "pixel";
		request.x = x;
		request.y = y;
		request.r = static_cast<int>(r);
		request.g = static_cast<int>(g);
		request.b = static_cast<int>(b);
		request.a = static_cast<int>(a);
		request.sorting_order = 2;
		request.pixel = true;
		renderQueue.push_back(request);
	}

	void addImages(std::vector<std::string> names) {
		for (std::string name : names) {
			if (imageCache.find(name) != imageCache.end()) {
				continue;
			}
			std::string fullPath = "resources/images/" + name + ".png";
			SDL_Texture* texture = IMG_LoadTexture(renderer, fullPath.c_str());
			SDL_Surface* surface = IMG_Load(fullPath.c_str());
			if (surface != nullptr) {
				surface = adjustSurfaceForColorblindness(surface, colorMode);
				texture = SDL_CreateTextureFromSurface(renderer, surface);
				SDL_FreeSurface(surface);
			}
			if (texture == nullptr) {
				std::cout << "error: missing image " << name;
				exit(0);
			}
			imageCache[name] = texture;
		}
	}

	void addTextImage(std::string name, SDL_Texture* texture) {
		if (imageCache.find(name) != imageCache.end()) {
			return;
		}
		imageCache[name] = texture;
	}

	void addImage(std::string name) {
		if (imageCache.find(name) != imageCache.end()) {
			return;
		}
		std::string fullPath = "resources/images/" + name + ".png";
		SDL_Texture* texture = IMG_LoadTexture(renderer, fullPath.c_str());
		SDL_Surface* surface = IMG_Load(fullPath.c_str());
		if (surface != nullptr) {
			surface = adjustSurfaceForColorblindness(surface, colorMode);
			texture = SDL_CreateTextureFromSurface(renderer, surface);
			SDL_FreeSurface(surface);
		}
		if (texture == nullptr) {
			std::cout << "error: missing image " << name;
			exit(0);
		}
		imageCache[name] = texture;
	}

	SDL_Texture* GetImage(std::string name) {
		if (imageCache.find(name) == imageCache.end()) {
			try {
				addImage(name);
				return imageCache[name];
			}
			catch (const std::exception& e) {
				std::cout << "error: missing image " << name;
				exit(0);
			}
		}
		else {
			return imageCache[name];
		}
	}

	void SetColorModeI(int mode) {
		colorMode = mode;
	}

	void SetColorMode(std::string mode){
		if (mode == "normal") {
			colorMode = 5;
		}
		else if (mode == "deuteranomaly") {
			colorMode = 0;
		}
		else if (mode == "protanopia") {
			colorMode = 1;
		}
		else if (mode == "deuteranopia") {
			colorMode = 2;
		}
		else {
			std::cerr << "Invalid color mode: " << mode << std::endl;
		}

		//overwrite colorMode in rendering.config, a json file
		rapidjson::Document doc;
		EngineUtils::ReadJsonFile("resources/rendering.config", doc);
		if(doc.HasMember("colorMode")){
			doc["colorMode"].SetInt(colorMode);
		}
		else {
			doc.AddMember("colorMode", colorMode, doc.GetAllocator());
		}
		//write to rendering.config using buffer writer amd ofstream
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		std::ofstream ofs("resources/rendering.config");
		ofs << buffer.GetString();
		ofs.close();
	}

	//comparator function that compares two ImageDrawRequests based on their sorting_order
	bool compRenderEls(const ImageDrawRequest& a, const ImageDrawRequest& b)
	{
		return a.sorting_order < b.sorting_order;
	}

	void RenderAndClearAllImages(float camera_zoom, glm::vec2 cameraPos, glm::ivec2 camDim, int clearR, int clearG, int clearB)
	{
		std::stable_sort(renderQueue.begin(), renderQueue.end(), compRenderEls);

		float zoom_factor = camera_zoom;

		for (auto& request : renderQueue)
		{
			const int pixels_per_meter = 100;
			glm::vec2 final_rendering_position;
			if (!request.scene_space) {
				final_rendering_position = glm::vec2(request.x, request.y);
				SDL_RenderSetScale(renderer, 1, 1);
			}
			else {
				SDL_RenderSetScale(renderer, zoom_factor, zoom_factor);
				final_rendering_position = glm::vec2(request.x, request.y) - cameraPos;
			}
			if (!request.pixel) {
				SDL_Texture* tex = GetImage(request.image_name);
				SDL_Rect tex_rect;
				SDL_QueryTexture(tex, NULL, NULL, &tex_rect.w, &tex_rect.h);

				// Apply scale
				int flip_mode = SDL_FLIP_NONE;
				if (request.scale_x < 0)
					flip_mode |= SDL_FLIP_HORIZONTAL;
				if (request.scale_y < 0)
					flip_mode |= SDL_FLIP_VERTICAL;

				float x_scale = std::abs(request.scale_x);
				float y_scale = std::abs(request.scale_y);

				tex_rect.w *= x_scale;
				tex_rect.h *= y_scale;

				SDL_Point pivot_point = { static_cast<int>(request.pivot_x * tex_rect.w), static_cast<int>(request.pivot_y * tex_rect.h) };

				glm::ivec2 cam_dimensions = camDim;

				if (request.scene_space) {
					tex_rect.x = static_cast<int>(final_rendering_position.x * pixels_per_meter + cam_dimensions.x * 0.5f * (1.0f / zoom_factor) - pivot_point.x);
					tex_rect.y = static_cast<int>(final_rendering_position.y * pixels_per_meter + cam_dimensions.y * 0.5f * (1.0f / zoom_factor) - pivot_point.y);
				}
				else {
					tex_rect.x = static_cast<int>(final_rendering_position.x - pivot_point.x);
					tex_rect.y = static_cast<int>(final_rendering_position.y - pivot_point.y);
				}
				// Apply tint / alpha to texture
				SDL_SetTextureColorMod(tex, request.r, request.g, request.b);
				SDL_SetTextureAlphaMod(tex, request.a);

				SDL_RenderCopyEx(renderer, tex, NULL, &tex_rect, request.rotation_degrees, &pivot_point, static_cast<SDL_RendererFlip>(flip_mode));
				if(request.scene_space) {
					SDL_RenderSetScale(renderer, zoom_factor, zoom_factor);
				}
				else {
					SDL_RenderSetScale(renderer, 1, 1);
				}

				// Remove tint / alpha from texture
				SDL_SetTextureColorMod(tex, 255, 255, 255);
				SDL_SetTextureAlphaMod(tex, 255);
			}
			else if (request.pixel) {
				//use RenderDrawPoint for pixel and RenderSetColor
				//set render draw blend mode to blend
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(renderer, request.r, request.g, request.b, request.a);
				SDL_RenderDrawPoint(renderer, request.x, request.y);
				SDL_SetRenderDrawColor(renderer, clearR, clearG, clearB, 255);
				SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
			}
		}

		SDL_RenderSetScale(renderer, 1, 1);

		renderQueue.clear();
	}


};
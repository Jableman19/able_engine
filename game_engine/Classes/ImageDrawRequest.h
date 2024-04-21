#pragma once
#include <string>
struct ImageDrawRequest {
	std::string image_name;
    float x = 0.0f;
	float y = 0.0f;
	int rotation_degrees = 0;
	float scale_x = 1.0f;
	float scale_y = 1.0f;
	float pivot_x = 0.0f;
	float pivot_y = 0.0f;
	int r = 255;
	int g = 255;
	int b = 255;
	int a = 255;
	int sorting_order = 0;
	bool scene_space = false;
	bool pixel = false;
};
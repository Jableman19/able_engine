#pragma once
#include <string>
class Text {
public:
	std::string text;
	int x;
	int y;
	std::string font_name;
	int font_size;
	int r;
	int g;
	int b;
	int a;

	Text(std::string text, int x, int y, std::string font_name, int font_size, int r, int g, int b, int a) {
		this->text = text;
		this->x = x;
		this->y = y;
		this->font_name = font_name;
		this->font_size = font_size;
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

};
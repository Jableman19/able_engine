#pragma once
#ifndef INPUT_H
#define INPUT_H

#include "SDL2/SDL.h"
#include "glm/glm.hpp"	
#include <unordered_map>
#include <vector>
#include "scancodes.h"

enum INPUT_STATE { INPUT_STATE_UP, INPUT_STATE_JUST_BECAME_DOWN, INPUT_STATE_DOWN, INPUT_STATE_JUST_BECAME_UP };

class Input
{
public:

	static void Init() {
		keyboard_states.clear();
		just_became_down_scancodes.clear();
		just_became_up_scancodes.clear();
		mouse_position = glm::vec2(0, 0);
		mouse_states.clear();
		just_became_down_buttons.clear();
		just_became_up_buttons.clear();
		//std::cout << "Input initialized" << std::endl;
	}// Call before main loop begins.

	static void ProcessEvent(const SDL_Event & e){
		if (e.type == SDL_KEYDOWN) {
			keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_DOWN;
			just_became_down_scancodes.push_back(e.key.keysym.scancode);
		}
		else if (e.type == SDL_KEYUP) {
			keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_UP;
			just_became_up_scancodes.push_back(e.key.keysym.scancode);
		}
		else if (e.type == SDL_MOUSEMOTION) {
			mouse_position = glm::vec2(e.motion.x, e.motion.y);
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN) {
			mouse_states[e.button.button] = INPUT_STATE_JUST_BECAME_DOWN;
			just_became_down_buttons.push_back(e.button.button);
		}
		else if (e.type == SDL_MOUSEBUTTONUP) {
			mouse_states[e.button.button] = INPUT_STATE_JUST_BECAME_UP;
			just_became_up_buttons.push_back(e.button.button);
		}
		else if (e.type == SDL_MOUSEWHEEL) {
			mouse_scroll_this_frame = e.wheel.preciseY;
		}
	}

	static void LateUpdate(){
		for (auto &scancode : just_became_down_scancodes) {
			if(keyboard_states[scancode] == INPUT_STATE_JUST_BECAME_DOWN)
				keyboard_states[scancode] = INPUT_STATE_DOWN;
		}
		just_became_down_scancodes.clear();

		for(auto &scancode : just_became_up_scancodes){
			if(keyboard_states[scancode] == INPUT_STATE_JUST_BECAME_UP)
				keyboard_states[scancode] = INPUT_STATE_UP;
		}
		just_became_up_scancodes.clear();

		for (auto &button : just_became_down_buttons) {
			if (mouse_states[button] == INPUT_STATE_JUST_BECAME_DOWN)
				mouse_states[button] = INPUT_STATE_DOWN;
		}
		just_became_down_buttons.clear();

		for (auto &button : just_became_up_buttons) {
			if (mouse_states[button] == INPUT_STATE_JUST_BECAME_UP)
				mouse_states[button] = INPUT_STATE_UP;
		}
		just_became_up_buttons.clear();

		mouse_scroll_this_frame = 0;
	}

	static glm::vec2 GetMousePosition() {
		return mouse_position;
	}
	static float GetMouseScrollDelta() {
		return mouse_scroll_this_frame;
	}
	//do all functions below but use scancode.h's __keycode_to_scancode map and take string as input
	//also add functions for mouse buttons
	static bool GetKey(std::string keycode) {
		try {
			SDL_Scancode scancode = __keycode_to_scancode.at(keycode);
			if (keyboard_states.find(scancode) == keyboard_states.end()) {
				return false;
			}
			return keyboard_states[scancode] == INPUT_STATE_DOWN || keyboard_states[scancode] == INPUT_STATE_JUST_BECAME_DOWN;
		}
		catch (const std::out_of_range& oor) {
			return false;
		}
	}
	static bool GetKeyDown(std::string keycode) {
		try {
			SDL_Scancode scancode = __keycode_to_scancode.at(keycode);
			if (keyboard_states.find(scancode) == keyboard_states.end()) {
				return false;
			}
			return keyboard_states[scancode] == INPUT_STATE_JUST_BECAME_DOWN;
		}
		catch (const std::out_of_range& oor) {
			return false;
		}
	}
	static bool GetKeyUp(std::string keycode) {
		try {
			SDL_Scancode scancode = __keycode_to_scancode.at(keycode);
			if (keyboard_states.find(scancode) == keyboard_states.end()) {
				return false;
			}
			return keyboard_states[scancode] == INPUT_STATE_JUST_BECAME_UP;
		}
		catch (const std::out_of_range& oor) {
			return false;
		}
	}
	static bool GetMouseButton(int button) {
		if (mouse_states.find(button) == mouse_states.end()) {
			return false;
		}
		return mouse_states[button] == INPUT_STATE_DOWN || mouse_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
	}
	static bool GetMouseButtonDown(int button) {
		if (mouse_states.find(button) == mouse_states.end()) {
			return false;
		}
		return mouse_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
	}
	static bool GetMouseButtonUp(int button) {
		if (mouse_states.find(button) == mouse_states.end()) {
			return false;
		}
		return mouse_states[button] == INPUT_STATE_JUST_BECAME_UP;
	}


private:
	static inline std::unordered_map<SDL_Scancode, INPUT_STATE> keyboard_states;
	static inline std::vector<SDL_Scancode> just_became_down_scancodes;
	static inline std::vector<SDL_Scancode> just_became_up_scancodes;

	static inline glm::vec2 mouse_position;
	static inline std::unordered_map<int, INPUT_STATE> mouse_states;
	static inline std::vector<int> just_became_down_buttons;
	static inline std::vector<int> just_became_up_buttons;

	static inline float mouse_scroll_this_frame = 0;
};

#endif

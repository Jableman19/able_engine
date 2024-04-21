#pragma once
#include "Actor.h"

class Player {
	public:
		Actor* actor;
		int health;
		int score;
		float speed = 0.02f;
		int lastFrameHit = -200;
		SDL_Texture *hp_texture;
		std::string hp_name = "";
		glm::vec2 new_pos = glm::vec2(0, 0);

		//constructor
		Player(Actor* _actor, int _health, int _score) :
			actor(_actor), health(_health), score(_score) {
			hp_texture = nullptr;
		}

		Player() {
			actor = nullptr;
			health = 3;
			score = 0;
			hp_texture = nullptr;
		}

		//getters
		Actor* getActor() { return actor; }
		int getHealth() { return health; }
		int getScore() { return score; }

		//setters
		void setActor(Actor* _actor) { actor = _actor; }
		void setHealth(int _health) { health = _health; }
		void setScore(int _score) { score = _score; }
		void setHpTexture(SDL_Texture* _hp_texture) { hp_texture = _hp_texture; }

};

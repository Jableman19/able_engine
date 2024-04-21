#include "SceneAPI.h"

namespace SceneAPI {
	Scene* scene = nullptr;
	EngineUtils engineUtils;
	rapidjson::Document jsonDoc;
	lua_State* L;

	void Load(std::string name) {
		scene->nextScene = name;
	}

	void DontDestroy(Actor actor) {
		std::vector<Actor*> actors = ActorDB::actorMap[actor.actor_name];
		for (int i = 0; i < actors.size(); i++) {
			if(actors[i]->id == actor.id) {
				actors[i]->dontDestroy = true;
			}
		}
	}

	std::string GetCurrent() {
		return scene->currentScene;
	}
}
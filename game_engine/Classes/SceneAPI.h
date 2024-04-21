#pragma once
#include <string>
#include "Scene.h"
namespace SceneAPI {
	extern Scene* scene;
	extern EngineUtils engineUtils;
	extern rapidjson::Document jsonDoc;
	extern lua_State* luaState;

	void Load(std::string name);

	std::string GetCurrent();

	void DontDestroy(Actor actor);
}
#pragma once
#include <memory>
#include <string>
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

class Component
{
public:
	explicit Component();

	bool IsEnabled() const;

	std::shared_ptr<luabridge::LuaRef> componentRef;
	std::string type;

	bool hasStart;
	bool hasUpdate;
	bool hasLateUpdate;
};
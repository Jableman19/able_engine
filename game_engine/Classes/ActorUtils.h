#pragma once
#include "glm/glm.hpp"
#include <string>
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"


//uint64_t packIvec2ToUint64(const glm::ivec2& vec) {
//	uint64_t xPart = static_cast<uint32_t>(vec.x) & 0xFFFFFFFF;
//	uint64_t yPart = static_cast<uint32_t>(vec.y) & 0xFFFFFFFF;
//	return (yPart << 32) | xPart;
//}

inline void ReportError(const std::string actorName, const luabridge::LuaException& e) {
	std::string error = e.what();
	std::replace(error.begin(), error.end(), '\\', '/');
	std::cout << "\033[31m" << actorName << " : " << error << "\033[0m\n";
}
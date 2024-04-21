#pragma once
#include <iostream>
#include <unordered_map>
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
#include <filesystem>
#include "box2d/box2d.h"

namespace ComponentDB {
		//map of component names to luabridge shared pointers
		inline std::map<std::string, std::shared_ptr<luabridge::LuaRef>> component_tables;
		inline int numCreated = 0;
		//lua state
		inline lua_State* L;
		

		inline static void CppLog(const std::string& message) {
			std::cout << message << std::endl;
		}

		inline static void CppLogError(const std::string& message) {
			std::cerr << message << std::endl;
		}

		//GetLuaState
		inline lua_State* GetLuaState() {
			return L;
		}

		inline void RegisterComponent(std::string luafile) {
			std::filesystem::path filePath(luafile);

			//load lua file
			if (luaL_dofile(L, luafile.c_str())) {
				std:: cout << "problem with lua file " << filePath.stem().string();
				exit(0);
			}

			std::string component_name = filePath.stem().string();
			component_tables.insert({ component_name,
				std::make_shared<luabridge::LuaRef>(
					luabridge::getGlobal(L, component_name.c_str()))
			});
			//add key to current component with value being the component key
		}

		inline void RegisterRigidbodyComponent() {
			//
		}

		inline void EstablishInheritance(luabridge::LuaRef& instance_table, luabridge::LuaRef& parent_table) {
			luabridge::LuaRef new_metatable = luabridge::newTable(L);
			new_metatable["__index"] = parent_table;
			instance_table.push(L);
			new_metatable.push(L);
			lua_setmetatable(L, -2);
			lua_pop(L, 1);
		}


		//Get and Set for L
		inline void SetLuaState(lua_State* state) {
			L = state;
		}


		//Get and Set for component tables
};
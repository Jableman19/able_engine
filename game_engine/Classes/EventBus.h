#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "Lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"

//Boom, event bus

struct EventRequests {
	std::string eventName;
	luabridge::LuaRef parent;
	luabridge::LuaRef callback;
};

namespace EventBus {
	std::unordered_map<std::string, std::vector<std::pair<luabridge::LuaRef, luabridge::LuaRef>>> eventMap;
	std::vector<EventRequests> eventRequests;
	std::vector<EventRequests> eventRequestsToRemove;

	void Subscribe(std::string eventName, luabridge::LuaRef parent, luabridge::LuaRef callback) {
		eventRequests.push_back({ eventName, parent, callback });
	}


	void Unsubscribe(std::string eventName, luabridge::LuaRef parent, luabridge::LuaRef callback) {
		eventRequestsToRemove.push_back({ eventName, parent, callback });
	}

	void Publish(std::string eventName, luabridge::LuaRef data) {
		if (eventMap.find(eventName) != eventMap.end()) {
			for (auto& pair : eventMap[eventName]) {
				pair.second(pair.first, data);
			}
		}
	}

	void ProcessRequests() {
		for (auto& request : eventRequests) {
			if (eventMap.find(request.eventName) != eventMap.end()) {
				eventMap[request.eventName].push_back({ request.parent, request.callback });
			}
			else {
				eventMap[request.eventName] = { { request.parent, request.callback } };
			}
		}
		eventRequests.clear();

		for (auto& request : eventRequestsToRemove) {
			if (eventMap.find(request.eventName) != eventMap.end()) {
				auto& vec = eventMap[request.eventName];
				for (int i = 0; i < vec.size(); i++) {
					if (vec[i].first == request.parent && vec[i].second == request.callback) {
						vec.erase(vec.begin() + i);
						break;
					}
				}
			}
		}
	}

}
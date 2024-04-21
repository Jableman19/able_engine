#pragma once
#include "ComponentDB.h"
#include <algorithm>
#include <vector>
#include "Raycasts.h"
#include "LuaBridge.h"
#include "WorldManager.h"

namespace Physics {

	luabridge::LuaRef Raycast(b2Vec2 pos, b2Vec2 dir, float dist) {
		if(WorldManager::world == nullptr || dist < 0)
			return luabridge::LuaRef(ComponentDB::GetLuaState());
		Raycasts newRaycast;
		//calculate the end point of the raycast
		dir.Normalize();
		dir *= dist;
		b2Vec2 endPoint = pos + dir;
		WorldManager::world->RayCast(&newRaycast, pos, endPoint);

		//sort the hits by distance
		std::sort(newRaycast.hits.begin(), newRaycast.hits.end(), [pos](HitResult a, HitResult b)
			{return b2DistanceSquared(pos, a.point) < b2DistanceSquared(pos, b.point); });

		if(newRaycast.hits.size() == 0)
			return luabridge::LuaRef(ComponentDB::GetLuaState());
		else {
			return luabridge::LuaRef(ComponentDB::GetLuaState(), newRaycast.hits[0]);
		}
	}

	luabridge::LuaRef RaycastAll(b2Vec2 pos, b2Vec2 dir, float dist) {
		if(WorldManager::world == nullptr || dist < 0)
			return luabridge::LuaRef(ComponentDB::GetLuaState());
		Raycasts newRaycast;
		//calculate the end point of the raycast
		dir.Normalize();
		dir *= dist;
		b2Vec2 endPoint = pos + dir;
		WorldManager::world->RayCast(&newRaycast, pos, endPoint);

		//sort the hits by distance
		std::sort(newRaycast.hits.begin(), newRaycast.hits.end(), [pos](HitResult a, HitResult b)
			{return b2DistanceSquared(pos, a.point) < b2DistanceSquared(pos, b.point); });

		luabridge::LuaRef hits = luabridge::newTable(ComponentDB::GetLuaState());
		if(newRaycast.hits.size() == 0)
			return hits;
		else {
			for(int i = 0; i < newRaycast.hits.size(); i++) {
				hits[i + 1] = newRaycast.hits[i];
			}
			return hits;
		}
	}


};
//#pragma once
//#include <string>
//#include <vector>
//#include <iostream>
//#include "Actor.h"
//
//
//struct Vec2Comparator {
//	bool operator()(const glm::vec2& a, const glm::vec2& b) const {
//		if (a.x < b.x) return true;
//		if (b.x < a.x) return false;
//		return a.y < b.y;
//	}
//};
//
//glm::vec2 getCollisionRegion(glm::vec2 actorPos, float maxCollide) {
//
//	//map the current actor pos to a region, where regions are as big as the largest collider dimension * 50!
//	float regionX = std::floor(actorPos.x / (maxCollide) * 50);
//	float regionY = std::floor(actorPos.y / (maxCollide) * 50);
//
//	return glm::vec2(regionX, regionY);
//
//}
//
//std::vector<glm::vec2> getCollisionRegions(Actor* actor, glm::vec2 rootPos, float maxCollide) {
//
//	////map the current actor collider vertices to the collision regions, where the region size is determined by maxCollide
//	//std::vector<glm::vec2> regions = {};
//	////get the region of the actor
//	//glm::vec2 region = getCollisionRegion(rootPos, maxCollide);
//
//	////determine our region by max width/height of collider or trigger
//	//float maxW = std::max(actor->colliderWidth, actor->triggerWidth);
//	//float maxH = std::max(actor->colliderHeight, actor->triggerHeight);
//
//	////get the collider dimensions
//	//float colliderWidth = maxCollide;
//	//float colliderHeight = maxCollide;
//	////get the top left corner of the col
//	//float topLeftX = rootPos.x - std::round(maxW * .5);
//	//float topLeftY = rootPos.y - std::round(maxH * .5);
//	////get the bottom right corner of the col
//	//float bottomRightX = rootPos.x + std::round(maxW * .5);
//	//float bottomRightY = rootPos.y + std::round(maxH * .5);
//	////get top right corner
//	//float topRightX = rootPos.x + std::round(maxW * .5);
//	//float topRightY = rootPos.y - std::round(maxH * .5);
//	////get bottom left corner
//	//float bottomLeftX = rootPos.x - std::round(maxW * .5);
//	//float bottomLeftY = rootPos.y + std::round(maxH * .5);
//
//	////get the top left region
//	//glm::vec2 topLeftRegion = getCollisionRegion(glm::vec2(topLeftX, topLeftY), maxCollide);
//	////get the bottom right region
//	//glm::vec2 bottomRightRegion = getCollisionRegion(glm::vec2(bottomRightX, bottomRightY), maxCollide);
//	////get the top right region
//	//glm::vec2 topRightRegion = getCollisionRegion(glm::vec2(topRightX, topRightY), maxCollide);
//	////get the bottom left region
//	//glm::vec2 bottomLeftRegion = getCollisionRegion(glm::vec2(bottomLeftX, bottomLeftY), maxCollide);
//	////add all regions to the vector
//	//regions.push_back(topLeftRegion);
//	//regions.push_back(bottomRightRegion);
//	//regions.push_back(topRightRegion);
//	//regions.push_back(bottomLeftRegion);
//	////remove duplicates
//	//std::sort(regions.begin(), regions.end(), Vec2Comparator());
//	//regions.erase(std::unique(regions.begin(), regions.end()), regions.end());
//
//	//return regions;
//	return {};
//}
//
//
//static std::string obtain_word_after_phrase(const std::string& input, const std::string& phrase) {
//	// Find the position of the phrase in the string
//	size_t pos = input.find(phrase);
//
//	// If phrase is not found, return an empty string
//	if (pos == std::string::npos) return "";
//
//	// Find the starting position of the next word (skip spaces after the phrase)
//	pos += phrase.length();
//	while (pos < input.size() && std::isspace(input[pos])) {
//		++pos;
//	}
//
//	// If we're at the end of the string, return an empty string
//	if (pos == input.size()) return "";
//
//	// Find the end position of the word (until a space or the end of the string)
//	size_t endPos = pos;
//	while (endPos < input.size() && !std::isspace(input[endPos])) {
//		++endPos;
//	}
//
//	// Extract and return the word
//	return input.substr(pos, endPos - pos);
//}
//
//float distanceBetweenActors(Actor* a, Actor* b) {
//	/*float x1 = a->position.x;
//	float y1 = a->position.y;
//	float x2 = b->position.x;
//	float y2 = b->position.y;
//	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));*/
//	return 0;
//}
//
//bool AABB_col(Actor* a, Actor* b, glm::vec2 aNewPos) {
//	//implement AABB collision detection where the pos is centered in the collider
//	//determine min and max X and Y of a and b, keep in mind, collider size is affected by scale
//	//if actorb is player
//	/*float aMinX = aNewPos.x - (a->colliderWidth * .5f) * std::abs(a->transform_scale_x);
//	float aMaxX = aNewPos.x + (a->colliderWidth * .5f) * std::abs(a->transform_scale_x);
//	float bMinX = b->position.x - (b->colliderWidth * .5f) * std::abs(b->transform_scale_x);
//	float bMaxX = b->position.x + (b->colliderWidth * .5f) * std::abs(b->transform_scale_x);
//	if (aMaxX > bMinX && aMinX < bMaxX) {
//		float aMinY = aNewPos.y - (a->colliderHeight * .5f) * std::abs(a->transform_scale_y);
//		float aMaxY = aNewPos.y + (a->colliderHeight * .5f) * std::abs(a->transform_scale_y);
//		float bMinY = b->position.y - (b->colliderHeight *.5f) * std::abs(b->transform_scale_y);
//		float bMaxY = b->position.y + (b->colliderHeight * .5f) * std::abs(b->transform_scale_y);
//		if (aMaxY > bMinY && aMinY < bMaxY) {
//			return true;
//		}
//	}*/
//
//	return false;
//
//}
//
//bool AABB_trig(Actor* a, Actor* b) {
//	//implement AABB trigger detection
//	/*float aMinX = a->position.x - (a->triggerWidth * .5f) * std::abs(a->transform_scale_x);
//	float aMaxX = a->position.x + (a->triggerWidth * .5f) * std::abs(a->transform_scale_x);
//	float bMinX = b->position.x - (b->triggerWidth * .5f) * std::abs(b->transform_scale_x);
//	float bMaxX = b->position.x + (b->triggerWidth * .5f) * std::abs(b->transform_scale_x);
//
//	if (aMaxX > bMinX && aMinX < bMaxX) {
//		float aMinY = a->position.y - (a->triggerHeight * .5f) * std::abs(a->transform_scale_y);
//		float aMaxY = a->position.y + (a->triggerHeight * .5f) * std::abs(a->transform_scale_y);
//		float bMinY = b->position.y - (b->triggerHeight * .5f) * std::abs(b->transform_scale_y);
//		float bMaxY = b->position.y + (b->triggerHeight * .5f) * std::abs(b->transform_scale_y);
//
//		if (aMaxY > bMinY && aMinY < bMaxY) {
//			return true;
//		}
//	}*/
//
//	return false;
//}

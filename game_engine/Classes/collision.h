#pragma once

class Actor;

class collision {
public:
	Actor* other;
	b2Vec2 point;
	b2Vec2 normal;
	b2Vec2 relative_velocity;
};
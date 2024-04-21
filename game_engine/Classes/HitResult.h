#pragma once
#include "box2d/box2d.h"

class Actor;

class HitResult {
public:
	Actor* actor;
	b2Vec2 point;
	b2Vec2 normal;
	bool is_trigger;

    // Default constructor
    HitResult() = default;

    // Copy constructor
    HitResult(const HitResult&) = default;

    // Copy assignment operator
    HitResult& operator=(const HitResult&) = default;

    // Destructor
    ~HitResult() = default;


};
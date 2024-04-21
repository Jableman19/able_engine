#pragma once
#include "Actor.h"

class ColliderListen : public b2ContactListener
{
public:
	//constructor
	ColliderListen();
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
};
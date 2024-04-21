#include "ColliderListen.h"


ColliderListen::ColliderListen() {
	WorldManager::world->SetContactListener(this);
};

void ColliderListen::BeginContact(b2Contact* contact) {
	//get separate fixtures
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();
	bool trigger = false;

	//if both fixtures are sensors
	if (fixtureA->IsSensor() && fixtureB->IsSensor()) {
		trigger = true;
	}
	else if (fixtureA->IsSensor() || fixtureB->IsSensor()) {
		return;
	}


	b2FixtureUserData data = fixtureA->GetUserData();
	Actor* actorA = reinterpret_cast<Actor*>(data.pointer);

	data = fixtureB->GetUserData();
	Actor* actorB = reinterpret_cast<Actor*>(data.pointer);

	b2WorldManifold worldManifold;
	contact->GetWorldManifold(&worldManifold);
	//get A b2Vec2 from worldManifold
	//get B b2Vec2 from worldManifold
	b2Vec2 point = worldManifold.points[0];
	//b2Vec2 Bpoint = worldManifold.points[1];

	b2Vec2 relativeVelocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
	//b2Vec2 relativeVelocityB = fixtureB->GetBody()->GetLinearVelocity() - fixtureA->GetBody()->GetLinearVelocity();



	if (trigger) {
		collision sendBcollision = { actorA, b2Vec2(-999.0f,-999.0f), b2Vec2(-999.0f,-999.0f), relativeVelocity };
		collision sendAcollision = { actorB, b2Vec2(-999.0f,-999.0f), b2Vec2(-999.0f,-999.0f), relativeVelocity };
		if (actorA && actorB) {
			actorA->OnTrigger(sendAcollision);
			actorB->OnTrigger(sendBcollision);
		}
	}
	else {

		collision sendBcollision = { actorA, point, worldManifold.normal, relativeVelocity };
		collision sendAcollision = { actorB, point, worldManifold.normal, relativeVelocity };
		if (actorA && actorB) {
			actorA->OnCollision(sendAcollision);
			actorB->OnCollision(sendBcollision);
		}
	}

}
void ColliderListen::EndContact(b2Contact* contact) {

	bool trigger = false;

	//get separate fixtures
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	//if both fixtures are sensors
	if(fixtureA->IsSensor() && fixtureB->IsSensor()) {
		trigger = true;
	}
	else if(fixtureA->IsSensor() || fixtureB->IsSensor()) {
		return;
	}


	b2FixtureUserData data = fixtureA->GetUserData();
	Actor* actorA = reinterpret_cast<Actor*>(data.pointer);

	data = fixtureB->GetUserData();
	Actor* actorB = reinterpret_cast<Actor*>(data.pointer);


	b2Vec2 relativeVelocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
	//b2Vec2 relativeVelocityB = fixtureB->GetBody()->GetLinearVelocity() - fixtureA->GetBody()->GetLinearVelocity();
	collision sendBcollision = { actorA, b2Vec2(-999.0f,-999.0f), b2Vec2(-999.0f,-999.0f), relativeVelocity };
	collision sendAcollision = { actorB, b2Vec2(-999.0f,-999.0f), b2Vec2(-999.0f,-999.0f), relativeVelocity };

	if (trigger) {
		if(actorA && actorB) {
			actorA->OnTriggerExit(sendAcollision);
			actorB->OnTriggerExit(sendBcollision);
		}
	}
	else {
	
		if (actorA && actorB) {
			actorA->OnCollisionExit(sendAcollision);
			actorB->OnCollisionExit(sendBcollision);
		}
	}
}
#pragma once
#include <string>
#include "DBs/WorldManager.h"
class Actor;

class Rigidbody {
public:
	std::string type = "Rigidbody";
	std::string key = "???";
	float x = 0.0f;
	float y = 0.0f;
	float width = 1.0f;
	float height = 1.0f;
	std::string body_type = "dynamic";
	b2Body* body;
	Actor* actor;
	bool enabled = true;
	bool precise = true;
	float gravity_scale = 1.0f;
	float density = 1.0f;
	float angular_friction = 0.3f;
	float rotation = 0.0f;
	bool has_collider = true;
	bool has_trigger = true;
	std::string collider_type = "box";
	float radius = 0.5f;
	float friction = 0.3f;
	float bounciness = 0.3f;
	std::string trigger_type = "box";
	float trigger_radius = 0.5f;
	float trigger_width = 1.0f;
	float trigger_height = 1.0f;

	Rigidbody() {};

	void Ready() {
		if (!WorldManager::rbs) {
			WorldManager::rbs = true;
			WorldManager::world = new b2World(b2Vec2(0.0f, 9.8f));
		}
	}

	b2Vec2 GetPosition() {
		if (body == nullptr) {
			return b2Vec2(x, y);
		}
		b2Vec2 pos = body->GetPosition();
		return pos;
	}

	float GetRotation() {
		if(body == nullptr) {
			return rotation;
		}
		float Radians = body->GetAngle();
		return Radians * (180.0f / b2_pi);
	}

	void AddForce(b2Vec2 force) {
		body->ApplyForceToCenter(force, true);
	}

	void SetVelocity(b2Vec2 vel) {
		body->SetLinearVelocity(vel);
	}

	void SetPosition(b2Vec2 pos) {
		if(body == nullptr) {
			x = pos.x;
			y = pos.y;
		}
		else {
			body->SetTransform(pos, body->GetAngle());
		}
	}

	void SetRotation(float degClockwise) {
		if(body == nullptr){
			rotation = degClockwise;
		}
		else {
			body->SetTransform(body->GetPosition(), degClockwise * (b2_pi / 180.0f));
		}
	}

	void SetAngularVelocity(float vel) {
		float radians = vel * (b2_pi / 180.0f);
		body->SetAngularVelocity(radians);
	}

	void SetGravityScale(float scale) {
		body->SetGravityScale(scale);
	}

	void SetUpDirection(b2Vec2 dir) {
		//normalize the direction vector
		dir.Normalize();
		float angle = atan2(dir.x, -dir.y);
		body->SetTransform(body->GetPosition(), angle);
	}

	void SetRightDirection(b2Vec2 dir) {
		//normalize the direction vector!
		dir.Normalize();
		float angle = atan2(dir.x, -dir.y) - (b2_pi / 2.0f);
		body->SetTransform(body->GetPosition(), angle);
	}

	b2Vec2 GetVelocity() {
		return body->GetLinearVelocity();
	}

	float GetAngularVelocity() {
		float degrees = body->GetAngularVelocity() * (180.0f / b2_pi);
		return degrees;
	}

	float GetGravityScale() {
		return body->GetGravityScale();
	}

	b2Vec2 GetUpDirection() {
		float angle = body->GetAngle();
		b2Vec2 result = b2Vec2(glm::sin(angle), -glm::cos(angle));
		result.Normalize();
		return result;
	}

	b2Vec2 GetRightDirection() {
		float angle = body->GetAngle();
		b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
		result.Normalize();
		return result;
	}

	void OnDestroy() {
		WorldManager::world->DestroyBody(body);
	}

	void OnStart() {

		b2BodyDef bodydef;
		if (body_type == "dynamic") {
			bodydef.type = b2_dynamicBody;
		}
		else if (body_type == "static") {
			bodydef.type = b2_staticBody;
		}
		else if (body_type == "kinematic") {
			bodydef.type = b2_kinematicBody;
		}

		bodydef.position.Set(x, y);
		bodydef.angle = rotation * (b2_pi / 180.0f);
		bodydef.gravityScale = gravity_scale;
		bodydef.bullet = precise;
		bodydef.angularDamping = angular_friction;
		body = WorldManager::world->CreateBody(&bodydef);

		if (!has_collider && !has_trigger)
		{
			b2PolygonShape phantom_shape;
			phantom_shape.SetAsBox(width * 0.5f, height * 0.5f);

			b2FixtureDef phantom_fixture_def;
			phantom_fixture_def.shape = &phantom_shape;
			phantom_fixture_def.density = density;

			// Because it is a sensor (with no callback event), no collisions will ever occur
			phantom_fixture_def.isSensor = true;
			body->CreateFixture(&phantom_fixture_def);
		}
		else {
			if (has_collider) {
				if (collider_type == "box") {
					b2PolygonShape shape;
					shape.SetAsBox(width * .5f, height * .5f);
					b2FixtureDef fixture;
					fixture.shape = &shape;
					fixture.density = density;
					fixture.friction = friction;
					fixture.restitution = bounciness;
					fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
					body->CreateFixture(&fixture);
				}
				else {
					b2CircleShape shape;
					shape.m_radius = radius;
					b2FixtureDef fixture;
					fixture.shape = &shape;
					fixture.density = density;
					fixture.friction = friction;
					fixture.restitution = bounciness;
					fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
					body->CreateFixture(&fixture);
				}
			}
			if(has_trigger) {
				if (trigger_type == "box") {
					b2PolygonShape trigger_shape;
					trigger_shape.SetAsBox(trigger_width * .5f, trigger_height * .5f);
					b2FixtureDef trigger_fixture;
					trigger_fixture.shape = &trigger_shape;
					trigger_fixture.density = density;
					trigger_fixture.isSensor = true;
					trigger_fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
					body->CreateFixture(&trigger_fixture);
				}
				else {
					b2CircleShape trigger_shape;
					trigger_shape.m_radius = trigger_radius;
					b2FixtureDef trigger_fixture;
					trigger_fixture.shape = &trigger_shape;
					trigger_fixture.density = density;
					trigger_fixture.isSensor = true;
					trigger_fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
					body->CreateFixture(&trigger_fixture);
				}
			}

		}


	}
};
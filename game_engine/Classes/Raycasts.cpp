#include "Raycasts.h"

float Raycasts::ReportFixture (b2Fixture* fixture, const b2Vec2& point,
        const b2Vec2& normal, float fraction)
 {
     //get actor pointer
     Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
     if (actor) {
        HitResult hit;
        hit.actor = actor;
        hit.point = point;
        hit.normal = normal;
        hit.is_trigger = fixture->IsSensor();
        hits.push_back(hit);
     }
     return 1;
    }
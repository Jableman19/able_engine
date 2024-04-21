#pragma once
#include <vector>
#include "HitResult.h"
#include "box2d/box2d.h"
class Raycasts : public b2RayCastCallback {
public:
    std::vector<HitResult> hits;

    virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
        const b2Vec2& normal, float fraction) override;

};
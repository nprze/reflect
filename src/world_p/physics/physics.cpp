#include "physics.h"
#include "world_p/components.h"
#include "world_p\ecs.h"

constexpr uint32_t substepCount = 10;

namespace rfct
{
    static flecs::query<gravityComponent, velocityComponent, positionComponent, dynamicBoxColliderComponent> gravityVelocityPositionBoxQuery;
    static flecs::query<staticBoxColliderComponent> staticBoxColliderQuery;
}
void rfct::createQueries(entity sceneEntity) {
    gravityVelocityPositionBoxQuery =
        ecs::get().query_builder<gravityComponent, velocityComponent, positionComponent, dynamicBoxColliderComponent>()
        .with(flecs::ChildOf, sceneEntity)
        .build();
    staticBoxColliderQuery =
        ecs::get().query_builder<staticBoxColliderComponent>()
        .with(flecs::ChildOf, sceneEntity)
        .build();
}
void rfct::cleanupQueries() {
    gravityVelocityPositionBoxQuery.~query();
    staticBoxColliderQuery.~query();
}


namespace rfct {
    bool checkForCollisionAABBAABB(dynamicBoxColliderComponent* a, staticBoxColliderComponent* b)
    {
        if (a->min.x > b->max.x || a->max.x < b->min.x) return false;
        if (a->min.y > b->max.y || a->max.y < b->min.y) return false;
        return true;
    }

    glm::vec2 ResolveAABBCollision(
        const dynamicBoxColliderComponent& dynamic,
        const staticBoxColliderComponent& staticCol
    ) {
        float dx1 = staticCol.max.x - dynamic.min.x;
        float dx2 = staticCol.min.x - dynamic.max.x;

        float dy1 = staticCol.max.y - dynamic.min.y;
        float dy2 = staticCol.min.y - dynamic.max.y;

        float overlapX = std::abs(dx1) < std::abs(dx2) ? dx1 : dx2;
        float overlapY = std::abs(dy1) < std::abs(dy2) ? dy1 : dy2;

        if (std::abs(overlapX) < std::abs(overlapY)) {
            return glm::vec2(overlapX, 0.0f);
        }
        else {
            return glm::vec2(0.0f, overlapY);
        }
    }
}

void rfct::updatePhysics(float dt, entity sceneEntity)
{
	static float accululator = 0.f;
	accululator += dt;
    if (accululator <= 1.f / 60.f) return;
    gravityVelocityPositionBoxQuery.each([&](flecs::entity ent, gravityComponent& gravity, velocityComponent& velocity, positionComponent& position, dynamicBoxColliderComponent& dynamicBox) {
        velocity.velocity += glm::vec2(0.f, 1.f) * gravity.gravity * dt;
		float substepTime = (1.f / 60.f) / (float)substepCount;
        for (uint32_t substep = 0; substep < substepCount; substep++) {
            glm::vec2 substepVelocity = velocity.velocity / (float)substepCount;
            position.position += substepVelocity * substepTime;
            dynamicBoxColliderComponent finalBoundingBox = { dynamicBox.min + position.position, dynamicBox.max + position.position };
            // TODO: not brute force
             staticBoxColliderQuery.each([&](flecs::entity e, staticBoxColliderComponent& staticBox) {
                if (checkForCollisionAABBAABB(&finalBoundingBox, &staticBox)) {
                    glm::vec2 resolution = ResolveAABBCollision(finalBoundingBox, staticBox);
                    position.position += resolution;

                    if (resolution.x != 0.0f) {
                        velocity.velocity.x = 0.0f;
                    }
                    if (resolution.y != 0.0f) {
                        velocity.velocity.y = 0.0f;
                    }
                }
                });
        }
    });
}

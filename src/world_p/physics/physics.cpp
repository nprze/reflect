#include "physics.h"
#include "world_p/components.h"
#include "world_p\ecs.h"

constexpr uint32_t substepCount = 10;
constexpr float dumping = 0.97f;

namespace rfct
{
    static flecs::query<gravityComponent, velocityComponent, positionComponent, dynamicBoxColliderComponent, collisionCallbackComponent> gravityVelocityPositionBoxQuery;
    static flecs::query<staticBoxColliderComponent> staticBoxColliderQuery;


    struct BVHnode {
        glm::vec2 min;
        glm::vec2 max;
        int left = -1;
        int right = -1;
        entity entity;
    };
}
void rfct::createQueries(entity sceneEntity) {
    gravityVelocityPositionBoxQuery =
        ecs::get().query_builder<gravityComponent, velocityComponent, positionComponent, dynamicBoxColliderComponent, collisionCallbackComponent>()
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

void rfct::updatePhysics(float dt)
{
    constexpr float deltaTime = 1.f / 60.f;
    static float accululator = 0.f;
    accululator += dt;
    while (accululator >= deltaTime){
        accululator -= deltaTime;
        gravityVelocityPositionBoxQuery.each([&](flecs::entity ent, gravityComponent& gravity, velocityComponent& velocity, positionComponent& position, dynamicBoxColliderComponent& dynamicBox, collisionCallbackComponent& callback) {
            velocity.velocity += glm::vec2(0.f, 1.f) * gravity.gravity * 20.f * deltaTime;
            velocity.velocity *= dumping;
            float substepTime = (deltaTime) / (float)substepCount;
            for (uint32_t substep = 0; substep < substepCount; substep++) {
                glm::vec2 substepVelocity = velocity.velocity / (float)substepCount;
                position.position += substepVelocity * substepTime;
                dynamicBoxColliderComponent finalBoundingBox = { dynamicBox.min + position.position, dynamicBox.max + position.position };
                // TODO: not brute force
                staticBoxColliderQuery.each([&](flecs::entity e, staticBoxColliderComponent& staticBox) {
                    if (checkForCollisionAABBAABB(&finalBoundingBox, &staticBox)) {
                        glm::vec2 resolution = ResolveAABBCollision(finalBoundingBox, staticBox);

                        callback.handler(ent, e, resolution);
                    }
                    });
            }

            });

    }
}

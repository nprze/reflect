#include "physics.h"
#include "world_p/ecs.h"
#include "renderer_p/debug/debug_draw.h"
#include "context.h"

#include <stack>
#include <limits>
#include <functional>
#include <algorithm>

constexpr float physicsScale = 100.f; // to make applied forces smaller for more readability
constexpr uint32_t substepCount = 10;
constexpr float dumping = 0.97f;

namespace rfct
{ 
    static flecs::query<gravityComponent, velocityComponent, positionComponent, dynamicBoxColliderComponent, staticObjCollisionCallbackComponent> gravityVelocityPositionBoxQuery;
    static flecs::query<positionComponent, dynamicBoxColliderComponent, dynamicObjCollisionCallbackComponent> dynamicObjectsQuery;
    static flecs::query<staticBoxColliderComponent> staticBoxColliderQuery;
    static flecs::query<dynamicBoxColliderComponent, positionComponent> dynamicBoxColliderQuery;
    static std::vector<BVHnode> StaticObjsBVHnodes;
    static std::vector<BVHnode> DynamicObjsBVHnodes;
}
// Queries helper
void rfct::createQueries(entity sceneEntity) {
    gravityVelocityPositionBoxQuery =
        ecs::get().query_builder<gravityComponent, velocityComponent, positionComponent, dynamicBoxColliderComponent, staticObjCollisionCallbackComponent>()
        .with(flecs::ChildOf, sceneEntity)
        .build();
    dynamicObjectsQuery =
        ecs::get().query_builder<positionComponent, dynamicBoxColliderComponent, dynamicObjCollisionCallbackComponent>()
        .with(flecs::ChildOf, sceneEntity)
        .build();
    staticBoxColliderQuery =
        ecs::get().query_builder<staticBoxColliderComponent>()
        .with(flecs::ChildOf, sceneEntity)
        .build();
    dynamicBoxColliderQuery =
        ecs::get().query_builder<dynamicBoxColliderComponent, positionComponent>()
        .with(flecs::ChildOf, sceneEntity)
        .build();
}

void rfct::cleanupQueries() {
    gravityVelocityPositionBoxQuery.~query();
    dynamicObjectsQuery.~query();
    staticBoxColliderQuery.~query();
    dynamicBoxColliderQuery.~query();
}

void rfct::buildStaticObjBVH()
{
    buildBVH<staticBoxColliderComponent>(staticBoxColliderQuery, &StaticObjsBVHnodes);
}

void rfct::buildDynamicObjBVH()
{
    buildDynamicBVH(dynamicBoxColliderQuery, &DynamicObjsBVHnodes);
}



// helper functions
namespace rfct {
    float SquaredDistanceToAABB(const glm::vec2& point, const glm::vec2& min, const glm::vec2& max) {
        float dx = std::max(std::max(min.x - point.x, 0.0f), point.x - max.x);
        float dy = std::max(std::max(min.y - point.y, 0.0f), point.y - max.y);
        return dx * dx + dy * dy;
    }

    uint32_t expandBits(uint16_t v) {
        uint32_t x = v;
        x = (x | (x << 8)) & 0x00FF00FF;
        x = (x | (x << 4)) & 0x0F0F0F0F;
        x = (x | (x << 2)) & 0x33333333;
        x = (x | (x << 1)) & 0x55555555;
        return x;
    }

    uint32_t getMortonCode(float x, float y) {
        x = glm::clamp(x, 0.0f, 1.0f);
        y = glm::clamp(y, 0.0f, 1.0f);

        uint16_t xi = static_cast<uint16_t>(x * 32767.0f);
        uint16_t yi = static_cast<uint16_t>(y * 32767.0f);

        return (expandBits(xi) << 1) | expandBits(yi);
    }

    std::array<glm::vec2, 2> getMinMax(const glm::vec2& amin, const glm::vec2& amax, const glm::vec2& bmin, const glm::vec2& bmax) {
        glm::vec2 totalMin = glm::min(amin, bmin);
        glm::vec2 totalMax = glm::max(amax, bmax);
        return { totalMin, totalMax };
    }

    struct Entry {
        uint32_t mortonCode;
        glm::vec2 min;
        glm::vec2 max;
        flecs::entity entity;
    };

    int createSubTree(std::vector<Entry> entries, uint32_t start, uint32_t end, std::vector<BVHnode>* BVHnodes) { // returns the index to the BVHnodes vector
        if (start == end) {
            // leaf
            BVHnode node{
                entries[start].min,
                entries[start].max,
                -1, -1,
                entries[start].entity
            };
            BVHnodes->push_back(node);
            return BVHnodes->size() - 1;
        }
        else {
            uint32_t middle = (start + end) / 2;
            int left = createSubTree(entries,start, middle, BVHnodes);
            int right = createSubTree(entries, middle+1, end, BVHnodes);
            std::array<glm::vec2, 2> nodeMinMax = getMinMax((*BVHnodes)[left].min, (*BVHnodes)[left].max, (*BVHnodes)[right].min, (*BVHnodes)[right].max);
            BVHnode node{
                nodeMinMax[0],
                nodeMinMax[1],
                left,
                right,
                entity()
            };
            BVHnodes->push_back(node);
            return BVHnodes->size() - 1;
        }
    }
}
template<typename T>
void rfct::buildBVH(flecs::query<T> qr, std::vector<BVHnode>* BVHnodes)
{
    BVHnodes->clear();
    glm::vec2 globalMin(FLT_MAX);
    glm::vec2 globalMax(-FLT_MAX);

    qr.each([&](flecs::entity e, T& box) {
        globalMin = glm::min(globalMin, box.min);
        globalMax = glm::max(globalMax, box.max);
        });

    glm::vec2 extent = globalMax - globalMin;

    std::vector<Entry> entries;

    qr.each([&](flecs::entity e, T& box) {
        glm::vec2 center = (box.min + box.max) * 0.5f;
        glm::vec2 normalized = (center - globalMin) / extent;
        uint32_t morton = getMortonCode(normalized.x, normalized.y);

        entries.push_back({ morton, box.min, box.max, e });
        });

    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        return a.mortonCode < b.mortonCode;
        });


    BVHnodes->reserve(entries.size() * 2 - 1);

    createSubTree(entries, 0, entries.size()-1, BVHnodes);
}

void rfct::buildDynamicBVH(flecs::query<dynamicBoxColliderComponent, positionComponent>& qr, std::vector<BVHnode>* BVHnodes)
{
    BVHnodes->clear();
    glm::vec2 globalMin(FLT_MAX);
    glm::vec2 globalMax(-FLT_MAX);

    qr.each([&](flecs::entity e, dynamicBoxColliderComponent& box, positionComponent& pos) {
        globalMin = glm::min(globalMin, box.min + pos.position);
        globalMax = glm::max(globalMax, box.max + pos.position);
        });

    glm::vec2 extent = globalMax - globalMin;

    std::vector<Entry> entries;

    qr.each([&](flecs::entity e, dynamicBoxColliderComponent& box, positionComponent& pos) {
        glm::vec2 center = (box.min + box.max) * 0.5f + pos.position;
        glm::vec2 normalized = (center - globalMin) / extent;
        uint32_t morton = getMortonCode(normalized.x, normalized.y);

        entries.push_back({ morton, box.min + pos.position, box.max + pos.position, e });
        });

    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        return a.mortonCode < b.mortonCode;
        });


    BVHnodes->reserve(entries.size() * 2 - 1);

    createSubTree(entries, 0, entries.size() - 1, BVHnodes);
}

// collision functions
namespace rfct {
    bool checkForCollisionAABBAABB(dynamicBoxColliderComponent* a, staticBoxColliderComponent* b)
    {
        return (a->min.x <= b->max.x && a->max.x >= b->min.x &&
            a->min.y <= b->max.y && a->max.y >= b->min.y);
    }
    bool checkForCollisionAABBAABB(const glm::vec2& aMin, const glm::vec2& aMax, const glm::vec2& bMin, const glm::vec2& bMax)
    {
        return (aMin.x <= bMax.x && aMax.x >= bMin.x &&
            aMin.y <= bMax.y && aMax.y >= bMin.y);
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

    void checkForCollision(BVHnode& node, dynamicBoxColliderComponent& bocCollider, staticObjCollisionCallbackComponent& callback, entity& dynamicEntity) {
        if (checkForCollisionAABBAABB(node.min, node.max, bocCollider.min, bocCollider.max)) {
            if (node.right < 0) 
            {
                // BVH leaf
                glm::vec2 resolution = ResolveAABBCollision(bocCollider, {node.min, node.max});
                callback.handler(dynamicEntity, node.entity, resolution);
            }
            else {
                checkForCollision(StaticObjsBVHnodes[node.right], bocCollider, callback, dynamicEntity);
                checkForCollision(StaticObjsBVHnodes[node.left], bocCollider, callback, dynamicEntity);
            }
        }
    }

    void checkForCollision(BVHnode& node, dynamicBoxColliderComponent& bocCollider, dynamicObjCollisionCallbackComponent& callback, entity& collidingEntity) {
        if (checkForCollisionAABBAABB(node.min, node.max, bocCollider.min, bocCollider.max)) {
            if (node.right < 0) 
            {
                // BVH leaf
                if (collidingEntity != node.entity) {
                    callback.handler(collidingEntity, node.entity);
                }
            }
            else {
                checkForCollision(DynamicObjsBVHnodes[node.right], bocCollider, callback, collidingEntity);
                checkForCollision(DynamicObjsBVHnodes[node.left], bocCollider, callback, collidingEntity);
            }
        }
    }
}

// BVH draw functions
namespace rfct {
    void drawAABB(const glm::vec2& min, const glm::vec2& max, uint32_t depth) {
        debugLine* lines = debugDraw::requestLines(4);
        float z_coord = depth / 100;
        lines[0].vertices[0].pos = { min.x, min.y, z_coord };
        lines[0].vertices[1].pos = { min.x, max.y, z_coord };

        lines[1].vertices[0].pos = { max.x, max.y, z_coord };
        lines[1].vertices[1].pos = { min.x, max.y, z_coord };

        lines[2].vertices[0].pos = { max.x, max.y, z_coord };
        lines[2].vertices[1].pos = { max.x, min.y, z_coord };

        lines[3].vertices[0].pos = { min.x, min.y, z_coord };
        lines[3].vertices[1].pos = { max.x, min.y, z_coord };

        switch (depth % 3) {
        case 0: {
            const glm::vec3 red = { 1.f,0.f,0.f };
            for (uint32_t i = 0; i < 4; i++) {
                lines[i].vertices[0].color = red;
                lines[i].vertices[1].color = red;
            }
            break;
        }
        case 1: {
            const glm::vec3 green = { 0.f,1.f,0.f };
            for (uint32_t i = 0; i < 4; i++) {
                lines[i].vertices[0].color = green;
                lines[i].vertices[1].color = green;
            }
            break;
        }
        case 2: { 
            const glm::vec3 blue = { 0.f,0.f,1.f };
            for (uint32_t i = 0; i < 4; i++) {
                lines[i].vertices[0].color = blue;
                lines[i].vertices[1].color = blue;
            }
            break;
        }
        }

    }
    void drawBVH(uint32_t depth, const BVHnode& start, std::vector<BVHnode>* nodes) {
        drawAABB(start.min, start.max, depth);
        if (!(start.right<0)) {
            drawBVH(depth+1, (*nodes)[start.left], nodes);
            drawBVH(depth+1, (*nodes)[start.right], nodes);
        }
    }
}

void rfct::updatePhysics(const frameContext* ctx)
{
    // drawBVH(0, DynamicObjsBVHnodes.back(), &DynamicObjsBVHnodes);
    for (uint32_t i = 0; i < ctx->fixedUpdateTimes;++i) {
        gravityVelocityPositionBoxQuery.each([&](flecs::entity ent, gravityComponent& gravity, velocityComponent& velocity, positionComponent& position, dynamicBoxColliderComponent& dynamicBox, staticObjCollisionCallbackComponent& callback) {
            if (gravity.gravityEnabled) {
                velocity.velocity.y += -gravity.gravity * fixedDeltaTime;
                velocity.velocity.y *= gravity.oneMinusAirResistance;
            }
            constexpr float substepTime = (fixedDeltaTime) / (float)substepCount;
            for (uint32_t substep = 0; substep < substepCount; substep++) {

                glm::vec2 substepVelocity = velocity.velocity / (float)substepCount;
                position.position += substepVelocity * physicsScale * substepTime;
                dynamicBoxColliderComponent finalBoundingBox = { dynamicBox.min + position.position, dynamicBox.max + position.position };
                checkForCollision(StaticObjsBVHnodes.back(), finalBoundingBox, callback, ent);
            }

            });

        // on dynamic objects do not update velocities
        dynamicObjectsQuery.each([&](flecs::entity ent, positionComponent& position, dynamicBoxColliderComponent& dynamicBox, dynamicObjCollisionCallbackComponent& callback) {
            constexpr float substepTime = (fixedDeltaTime) / (float)substepCount;
            for (uint32_t substep = 0; substep < substepCount; substep++) {
                dynamicBoxColliderComponent finalBoundingBox = { dynamicBox.min + position.position, dynamicBox.max + position.position };
                checkForCollision(DynamicObjsBVHnodes.back(), finalBoundingBox, callback, ent);
            }

            });
    }
}


entity rfct::findTheNearestVineToPlayer(entity player)
{
    glm::vec2 point = player.get<positionComponent>()->position;
    std::vector<BVHnode>& bvh = DynamicObjsBVHnodes;
    entity nearestEntity{};
    float nearestDistSq = std::numeric_limits<float>::max();

    std::stack<int> stack;
    stack.push(static_cast<int>(bvh.size()) - 1); // Root

    while (!stack.empty()) {
        int nodeIndex = stack.top();
        stack.pop();
        const BVHnode& node = bvh[nodeIndex];

        float distSq = SquaredDistanceToAABB(point, node.min, node.max);
        if (distSq >= nearestDistSq) continue; // This node can’t improve

        if (node.left == -1 && node.right == -1) {
            if (node.entity.get<dynamicObjectTypeComponent>()->type == dynamicObjectType::Vine) {
                float entityDistSq = 0.0f;
                entityDistSq = distSq;

                if (entityDistSq < nearestDistSq) {
                    nearestDistSq = entityDistSq;
                    nearestEntity = node.entity;
                }
            }
        }
        else {
            int left = node.left;
            int right = node.right;

            if (left != -1 && right != -1) {
                float leftDist = SquaredDistanceToAABB(point, bvh[left].min, bvh[left].max);
                float rightDist = SquaredDistanceToAABB(point, bvh[right].min, bvh[right].max);

                if (leftDist < rightDist) {
                    if (rightDist < nearestDistSq) stack.push(right);
                    if (leftDist < nearestDistSq) stack.push(left);
                }
                else {
                    if (leftDist < nearestDistSq) stack.push(left);
                    if (rightDist < nearestDistSq) stack.push(right);
                }
            }
            else {
                if (left != -1 && SquaredDistanceToAABB(point, bvh[left].min, bvh[left].max) < nearestDistSq)
                    stack.push(left);
                if (right != -1 && SquaredDistanceToAABB(point, bvh[right].min, bvh[right].max) < nearestDistSq)
                    stack.push(right);
            }
        }
    }return nearestEntity;
}
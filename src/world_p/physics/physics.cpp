#include "physics.h"
#include "world_p/components.h"
#include "world_p\ecs.h"
#include "renderer_p\debug\debug_draw.h"

constexpr uint32_t substepCount = 10;
constexpr float dumping = 0.97f;

namespace rfct
{
    struct BVHnode {
        glm::vec2 min;
        glm::vec2 max;
        int left = -1;
        int right = -1;
        entity entity;
    };
    static flecs::query<gravityComponent, velocityComponent, positionComponent, dynamicBoxColliderComponent, collisionCallbackComponent> gravityVelocityPositionBoxQuery;
    static flecs::query<staticBoxColliderComponent> staticBoxColliderQuery;
    static std::vector<BVHnode> BVHnodes;
}
// Queries helper
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

// BVH build helper functions
namespace rfct {
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

    int createSubTree(std::vector<Entry> entries, uint32_t start, uint32_t end) { // returns the index to the BVHnodes vector
        if (start == end) {
            // leaf
            BVHnode node{
                entries[start].min,
                entries[start].max,
                -1, -1,
                entries[start].entity
            };
            BVHnodes.push_back(node);
            return BVHnodes.size() - 1;
        }
        else {
            uint32_t middle = (start + end) / 2;
            int left = createSubTree(entries,start, middle);
            int right = createSubTree(entries, middle+1, end);
            std::array<glm::vec2, 2> nodeMinMax = getMinMax(BVHnodes[left].min, BVHnodes[left].max, BVHnodes[right].min, BVHnodes[right].max);
            BVHnode node{
                nodeMinMax[0],
                nodeMinMax[1],
                left,
                right,
                entity()
            };
            BVHnodes.push_back(node);
            return BVHnodes.size() - 1;
        }
    }
}

void rfct::buildBVH()
{

    glm::vec2 globalMin(FLT_MAX);
    glm::vec2 globalMax(-FLT_MAX);

    staticBoxColliderQuery.each([&](flecs::entity e, staticBoxColliderComponent& box) {
        globalMin = glm::min(globalMin, box.min);
        globalMax = glm::max(globalMax, box.max);
        });

    glm::vec2 extent = globalMax - globalMin;

    std::vector<Entry> entries;

    staticBoxColliderQuery.each([&](flecs::entity e, staticBoxColliderComponent& box) {
        glm::vec2 center = (box.min + box.max) * 0.5f;
        glm::vec2 normalized = (center - globalMin) / extent;
        uint32_t morton = getMortonCode(normalized.x, normalized.y);

        entries.push_back({ morton, box.min, box.max, e });
        });

    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        return a.mortonCode < b.mortonCode;
        });


    BVHnodes.reserve(entries.size() * 2 - 1);

    createSubTree(entries, 0, entries.size()-1);
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

    void checkForCollision(BVHnode& node, dynamicBoxColliderComponent& bocCollider, collisionCallbackComponent& callback, entity& dynamicEntity) {
        if (checkForCollisionAABBAABB(node.min, node.max, bocCollider.min, bocCollider.max)) {
            if (node.right < 0)
            {
                glm::vec2 resolution = ResolveAABBCollision(bocCollider, {node.min, node.max});
                callback.handler(dynamicEntity, node.entity, resolution);
            }
            else {
                checkForCollision(BVHnodes[node.right], bocCollider, callback, dynamicEntity);
                checkForCollision(BVHnodes[node.left], bocCollider, callback, dynamicEntity);
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
    void drawBVH(uint32_t depth, const BVHnode& start) {
        drawAABB(start.min, start.max, depth);
        if (!(start.right<0)) {
            drawBVH(depth+1, BVHnodes[start.left]);
            drawBVH(depth+1, BVHnodes[start.right]);
        }
    }
}

void rfct::updatePhysics(float dt)
{
    constexpr float deltaTime = 1.f / 60.f;
    static float accululator = 0.f;
    accululator += dt;
    drawBVH(0, BVHnodes.back());
    while (accululator >= deltaTime){
        accululator -= deltaTime;
        gravityVelocityPositionBoxQuery.each([&](flecs::entity ent, gravityComponent& gravity, velocityComponent& velocity, positionComponent& position, dynamicBoxColliderComponent& dynamicBox, collisionCallbackComponent& callback) {
            if (gravity.gravityEnabled) {
                velocity.velocity += glm::vec2(0.f, -1.f) * gravity.gravity * 20.f * deltaTime;
                velocity.velocity *= dumping;
            }
            float substepTime = (deltaTime) / (float)substepCount;
            for (uint32_t substep = 0; substep < substepCount; substep++) {
                if (gravity.gravityEnabled) {
                    glm::vec2 substepVelocity = velocity.velocity / (float)substepCount;
                    position.position += substepVelocity * substepTime;
                }
                dynamicBoxColliderComponent finalBoundingBox = { dynamicBox.min + position.position, dynamicBox.max + position.position };
                checkForCollision(BVHnodes.back(), finalBoundingBox, callback, ent);
            }

            });

    }
}

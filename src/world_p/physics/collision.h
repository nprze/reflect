#pragma once
#include "world_p/components.h"
#include "physics.h"

namespace rfct {
    inline glm::vec2 nearestPointOnAABB(const glm::vec2& point, const glm::vec2& AABBMin, const glm::vec2& AABBMax)
    {
        return glm::vec2(glm::clamp(point.x, AABBMin.x, AABBMax.x), glm::clamp(point.y, AABBMin.y, AABBMax.y));
    }
    inline bool checkForCollisionAABBAABB(dynamicBoxColliderComponent* a, staticBoxColliderComponent* b)
    {
        return (a->min.x <= b->max.x && a->max.x >= b->min.x &&
            a->min.y <= b->max.y && a->max.y >= b->min.y);
    }
    inline bool checkForCollisionAABBAABB(const glm::vec2& aMin, const glm::vec2& aMax, const glm::vec2& bMin, const glm::vec2& bMax)
    {
        return (aMin.x <= bMax.x && aMax.x >= bMin.x &&
            aMin.y <= bMax.y && aMax.y >= bMin.y);
    }
    inline bool checkCollisionAABBCircle(const glm::vec2& aMin, const glm::vec2& aMax, const glm::vec2& circleCenter, float radius) {
        glm::vec2 nearest = nearestPointOnAABB(circleCenter, aMin, aMax);
        float distSq = glm::dot(nearest - circleCenter, nearest - circleCenter);
        return distSq <= radius * radius;
    }
    inline bool checkIntersectRayAABB(const glm::vec2& ro, const glm::vec2& rd, const glm::vec2& aabbMin, const glm::vec2& aabbMax)
    {
        glm::vec2 invDir;
        invDir.x = (rd.x != 0.0f) ? 1.0f / rd.x : std::numeric_limits<float>::infinity();
        invDir.y = (rd.y != 0.0f) ? 1.0f / rd.y : std::numeric_limits<float>::infinity();

        float t1 = (aabbMin.x - ro.x) * invDir.x;
        float t2 = (aabbMax.x - ro.x) * invDir.x;
        float tmin = std::min(t1, t2);
        float tmax = std::max(t1, t2);

        float ty1 = (aabbMin.y - ro.y) * invDir.y;
        float ty2 = (aabbMax.y - ro.y) * invDir.y;
        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        if (tmax < 0.0f) return false;
        if (tmin > tmax) return false;
        return true;
    }
    inline bool checkIntersectSegmentAABB(const glm::vec2& p0,const glm::vec2& p1,const glm::vec2& aabbMin, const glm::vec2& aabbMax) {
        glm::vec2 d = p1 - p0;
        glm::vec2 invDir(
            d.x != 0.0f ? 1.0f / d.x : std::numeric_limits<float>::infinity(),
            d.y != 0.0f ? 1.0f / d.y : std::numeric_limits<float>::infinity()
        );

        float t1 = (aabbMin.x - p0.x) * invDir.x;
        float t2 = (aabbMax.x - p0.x) * invDir.x;
        float tmin = std::min(t1, t2);
        float tmax = std::max(t1, t2);

        float ty1 = (aabbMin.y - p0.y) * invDir.y;
        float ty2 = (aabbMax.y - p0.y) * invDir.y;
        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        if (tmin > tmax) return false;

        if (tmax < 0.0f) return false; 
        if (tmin > 1.0f) return false; 

        return true;
    }

    glm::vec2 ResolveAABBCollision(const dynamicBoxColliderComponent& dynamic, const staticBoxColliderComponent& staticCol);
    glm::vec2 ResolveAABBCollision(const glm::vec2& aMin, const glm::vec2& aMax, const glm::vec2& bMin, const glm::vec2& bMax); // returns what should be applied to a to resolve :)
    glm::vec2 ResolveAABBCircleCollision(const dynamicCircleColliderComponent& dynamic, const staticBoxColliderComponent& staticCol);


    bool checkRayStatic(BVHnode& node, const glm::vec2& rayStart, const glm::vec2& rayEnd);
}
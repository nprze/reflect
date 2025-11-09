#include "collision.h"

glm::vec2 rfct::ResolveAABBCollision(const dynamicBoxColliderComponent& dynamic, const staticBoxColliderComponent& staticCol)
{
    float overlapLeft = dynamic.max.x - staticCol.min.x;
    float overlapRight = staticCol.max.x - dynamic.min.x;
    float overlapDown = dynamic.max.y - staticCol.min.y;
    float overlapUp = staticCol.max.y - dynamic.min.y;

    float resolveX = 0.0f;
    if (overlapLeft > 0.0f || overlapRight > 0.0f) {
        if (overlapLeft < overlapRight) {
            resolveX = -overlapLeft;
        }
        else {
            resolveX = overlapRight;
        }
    }

    float resolveY = 0.0f;
    if (overlapDown > 0.0f || overlapUp > 0.0f) {
        if (overlapDown < overlapUp) {
            resolveY = -overlapDown;
        }
        else {
            resolveY = overlapUp;
        }
    }

    if (std::abs(resolveX) < std::abs(resolveY)) {
        return glm::vec2(resolveX, 0.0f);
    }
    else {
        return glm::vec2(0.0f, resolveY);
    }
}

glm::vec2 rfct::ResolveAABBCollision(const glm::vec2& aMin, const glm::vec2& aMax, const glm::vec2& bMin, const glm::vec2& bMax)
{
    float overlapLeft = aMax.x - bMin.x;
    float overlapRight = bMax.x - aMin.x;
    float overlapDown = aMax.y - bMin.y;
    float overlapUp = bMax.y - aMin.y;

    float resolveX = 0.0f;
    if (overlapLeft > 0.0f || overlapRight > 0.0f) {
        if (overlapLeft < overlapRight) {
            resolveX = -overlapLeft;
        }
        else {
            resolveX = overlapRight;
        }
    }

    float resolveY = 0.0f;
    if (overlapDown > 0.0f || overlapUp > 0.0f) {
        if (overlapDown < overlapUp) {
            resolveY = -overlapDown;
        }
        else {
            resolveY = overlapUp;
        }
    }

    if (std::abs(resolveX) < std::abs(resolveY)) {
        return glm::vec2(resolveX, 0.0f);
    }
    else {
        return glm::vec2(0.0f, resolveY);
    }
}

bool rfct::checkRayStatic(BVHnode& node, const glm::vec2& rayStart, const glm::vec2& rayEnd)
{

    if (checkIntersectSegmentAABB(rayStart, rayEnd, node.min, node.max)) {
        if (node.right < 0)
        {
            return true;
        }
        else {
            bool returnVal = false;
            if (checkRayStatic(StaticObjsBVHnodes[node.right], rayStart, rayEnd)) returnVal =  true;
            if (checkRayStatic(StaticObjsBVHnodes[node.left], rayStart, rayEnd)) returnVal =  true;
            return returnVal;
        }
    }return false;
}
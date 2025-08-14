#include "cigarettes.h"
#include "world_p/components.h"
#include "context.h"
#include "world_p/scene.h"
#include "world_p/transform.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace {
    constexpr glm::vec2 gravity{ 0.f, -5.f };
    constexpr float bounceDamping = 0.3f;
    constexpr float linearDamping = 0.9f;
    constexpr float angularDamping = 0.98f;
    constexpr float torqueScale = 0.05f; // tweak for realism
}

std::vector<rfct::Vertex> cigaretteVertices;
void rfct::initCigaretteVertices()
{
    cigaretteVertices.resize(12);


    constexpr float between = 0.10f;
    constexpr float rest = .15f;

    glm::vec3 blue = { 0.f,0.f, 1.f };
    glm::vec3 black = { 0.f,0.f, 0.f };
    // background triangles
    glm::vec3 bg0 = glm::vec3(-rest, rest, 0);
    glm::vec3 bg1 = glm::vec3(rest, rest, 0);
    glm::vec3 bg2 = glm::vec3(-rest, -rest, 0);
    glm::vec3 bg3 = glm::vec3(rest, -rest, 0);


    cigaretteVertices[0].pos = bg0;
    cigaretteVertices[1].pos = bg1;
    cigaretteVertices[2].pos = bg2;

    cigaretteVertices[3].pos = bg3;
    cigaretteVertices[4].pos = bg1;
    cigaretteVertices[5].pos = bg2;

    for (uint8_t i = 0; i < 6; ++i) {
        cigaretteVertices[i].color = black;
    }


    // color triangles
    glm::vec3 v0 = glm::vec3(-between, between, 0);
    glm::vec3 v1 = glm::vec3(between, between, 0);
    glm::vec3 v2 = glm::vec3(-between, -between, 0);
    glm::vec3 v3 = glm::vec3(between, -between, 0);


    cigaretteVertices[6].pos = v2;
    cigaretteVertices[7].pos = v1;
    cigaretteVertices[8].pos = v3;

    cigaretteVertices[9].pos = v0;
    cigaretteVertices[10].pos = v1;
    cigaretteVertices[11].pos = v2;


    for (uint8_t i = 6; i < 12; ++i) {
        cigaretteVertices[i].color = blue;
    }

}

void rfct::onCollision_Cigarette_StaticObj(entity cigarette, entity collidedWith, glm::vec2 resolution)
{
    auto* rot = cigarette.get_mut<rotationComponent>();
    auto* pos = cigarette.get_mut<positionComponent>();
    auto* vel = cigarette.get_mut<velocityComponent>();
    auto* angVel = cigarette.get_mut<angularVelocityComponent>();

    auto* collidedWithAABB = collidedWith.get<staticBoxColliderComponent>();
    

    pos->position += resolution;
}

void rfct::updateCigarette(entity cigaretteEntity, const frameContext* fc) {
    auto* pos = cigaretteEntity.get_mut<positionComponent>();
    auto* velocity = cigaretteEntity.get_mut<velocityComponent>();
    auto* angVel = cigaretteEntity.get_mut<angularVelocityComponent>();
    auto* rotation = cigaretteEntity.get_mut<rotationComponent>(); // assuming degrees or radians

    for (uint8_t i = 0; i < fc->fixedUpdateTimes; ++i) {
        // Apply gravity
        velocity->velocity += gravity * fixedDeltaTime;

        // Apply linear damping
        velocity->velocity *= linearDamping;

        // Update position
        pos->position += velocity->velocity * fixedDeltaTime;

        // Update angular motion
        angVel->zAngularVelocity *= angularDamping;
        rotation->rotation.z += angVel->zAngularVelocity * fixedDeltaTime; // radians
    }

    // Update render matrix
    glm::mat4 mat = getModelMatrixFromEntity(cigaretteEntity);
    fc->scene->getRenderData().updateMat(
        fc,
        cigaretteEntity.get<dynamicSSBOIndexComponent>()->indexInSSBO,
        &mat
    );
}

entity rfct::constructCigarette(const frameContext* fc, const entity entityPlayer, const bool facingRight)
{
    glm::mat4 transMat = glm::translate(glm::mat4(1.f), glm::vec3(entityPlayer.get<positionComponent>()->position, 0.f));
    entity newCigarette = fc->scene->createDynamicRenderingEntity(&cigaretteVertices, &transMat);
    newCigarette.set<positionComponent>({ entityPlayer.get<positionComponent>()->position });
    newCigarette.set<staticObjCollisionCallbackComponent>({ onCollision_Cigarette_StaticObj });
    newCigarette.set<rotationComponent>({});
    newCigarette.set<angularVelocityComponent>({ 3.f });
    newCigarette.set<scaleComponent>({});
    newCigarette.set<gravityComponent>({ 0.97f, false, 5.f });
    newCigarette.set<velocityComponent>({ .5f * glm::vec2{facingRight ? -1.f : 1.f, 1.f} });
    newCigarette.set<dynamicObjectTypeComponent>({ dynamicObjectType::Cigarette });
    newCigarette.set<dynamicBoxColliderComponent>({});
    constructCigaretteBoundingBox(newCigarette);
    return newCigarette;
}

void rfct::constructCigaretteBoundingBox(entity cigarette)
{
    dynamicBoxColliderComponent* boc = cigarette.get_mut<dynamicBoxColliderComponent>();
    const rotationComponent* rot = cigarette.get<rotationComponent>();

    boc->max = { FLT_MIN, FLT_MIN };
    boc->min = { FLT_MAX, FLT_MAX };

    // Create rotation matrix from Euler angles (XYZ order, adjust if your system differs)
    glm::mat4 rotationMat(1.0f);
    rotationMat = glm::rotate(rotationMat, rot->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // pitch
    rotationMat = glm::rotate(rotationMat, rot->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // yaw
    rotationMat = glm::rotate(rotationMat, rot->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // roll

    for (uint8_t i = 0; i < 4; ++i) {
        glm::vec4 rotatedPos = rotationMat * glm::vec4(cigaretteVertices[i].pos, 1.0f);

        boc->min.x = std::min(rotatedPos.x, boc->min.x);
        boc->min.y = std::min(rotatedPos.y, boc->min.y);

        boc->max.x = std::max(rotatedPos.x, boc->max.x);
        boc->max.y = std::max(rotatedPos.y, boc->max.y);
    }


}

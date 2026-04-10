#include "cigarettes.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "world_p/components.h"
#include "world_p/scene.h"
#include "world_p/transform.h"
#include "world_p/object_components.h"
#include "world_p/decors/smokes.h"

constexpr float angularDamping = 0.94f;
constexpr glm::vec2 gravity{ 0.f, -5.f };
constexpr float linearDamping = 0.9f;
constexpr float betweenVer = 0.05f;
constexpr float betweenHor = 0.11f;
constexpr float restVer = .07f;
constexpr float restHor = .13f;
constexpr uint8_t cigarettesMaxCount = 4;

std::vector<entity> cigarettesVec;
uint8_t lastCigaretteIndex = 0;
std::vector<rfct::Vertex> cigaretteModelVertices;

namespace rfct {
    void onCollision_Cigarette_StaticObj(entity cigarette, entity collidedWith, glm::vec2 resolution) {
        RFCT_PROFILE_FUNCTION();
        entt::registry& reg = ecs::get();
        auto& rot = reg.get<rotationComponent>(cigarette);
        auto& pos = reg.get<positionComponent>(cigarette);
        auto& vel = reg.get<velocityComponent>(cigarette);
        auto& angVel = reg.get<angularVelocityComponent>(cigarette);
        auto& collidedWithAABB = reg.get<staticBoxColliderComponent>(collidedWith);

        pos.position += resolution;

        if (!reg.get<cigaretteUpdateComponent>(cigarette).spawnedSmoke) {
            reg.get<cigaretteUpdateComponent>(cigarette).spawnedSmoke = true;
            reg.get<cigaretteUpdateComponent>(cigarette).shouldSpawnSmoke = true;
        }
    }
}

namespace rfct {
    void cigarettes::initSystem() {
        RFCT_PROFILE_FUNCTION();
        cigarettesVec.assign(cigarettesMaxCount, entt::null);

        // vertices
        cigaretteModelVertices.resize(12);


        glm::vec3 white = { 0.6f, 0.6f, 0.6f };
        glm::vec3 black = { 0.f,0.f, 0.f };
        // background triangles
        glm::vec3 bg0 = glm::vec3(-restHor, restVer, 0);
        glm::vec3 bg1 = glm::vec3(restHor, restVer, 0);
        glm::vec3 bg2 = glm::vec3(-restHor, -restVer, 0);
        glm::vec3 bg3 = glm::vec3(restHor, -restVer, 0);


        cigaretteModelVertices[0].pos = bg0;
        cigaretteModelVertices[1].pos = bg1;
        cigaretteModelVertices[2].pos = bg2;

        cigaretteModelVertices[3].pos = bg3;
        cigaretteModelVertices[4].pos = bg1;
        cigaretteModelVertices[5].pos = bg2;

        for (uint8_t i = 0; i < 6; ++i) {
            cigaretteModelVertices[i].color = black;
        }

        // color triangles
        glm::vec3 v0 = glm::vec3(-betweenHor, betweenVer, 0);
        glm::vec3 v1 = glm::vec3(betweenHor, betweenVer, 0);
        glm::vec3 v2 = glm::vec3(-betweenHor, -betweenVer, 0);
        glm::vec3 v3 = glm::vec3(betweenHor, -betweenVer, 0);


        cigaretteModelVertices[6].pos = v2;
        cigaretteModelVertices[7].pos = v1;
        cigaretteModelVertices[8].pos = v3;

        cigaretteModelVertices[9].pos = v0;
        cigaretteModelVertices[10].pos = v1;
        cigaretteModelVertices[11].pos = v2;


        for (uint8_t i = 6; i < 12; ++i) {
            cigaretteModelVertices[i].color = white;
        }

    }

    void cigarettes::resetLevel(const frameContext* ctx) {
        RFCT_PROFILE_FUNCTION();
        for (entity& e : cigarettesVec) {
            if(ecs::get().valid(e))
                ctx->scene->deleteDynamicEntity(e);
        }
    }

    void cigarettes::updateVisuals(const frameContext* ctx) {
        RFCT_PROFILE_FUNCTION();
        renderData& rd = ctx->scene->getRenderData();
        auto gravityVelocityPositionBoxQuery = ecs::get().view<cigaretteUpdateComponent, dynamicSSBOIndexComponent, positionComponent, rotationComponent, scaleComponent>();
        for (auto [ent, upd, ssboData, pos, rot, sc] : gravityVelocityPositionBoxQuery.each()) {
            glm::mat4 mat = getModelMatrix(pos, rot, sc);
            rd.updateMat(ctx, ssboData.indexInSSBO, &mat);
        };
    }

    void cigarettes::updateSystem(frameContext* ctx) {
        RFCT_PROFILE_FUNCTION();
        renderData& rd = ctx->scene->getRenderData();
        auto cigaretteComponentsQuery = ecs::get().view<cigaretteUpdateComponent, positionComponent, velocityComponent, angularVelocityComponent, rotationComponent>();
        for (auto [cigaretteEntity, update, pos, velocity, angVel, rotation] : cigaretteComponentsQuery.each()) {
            if (update.shouldBeUpdated) {
                // Apply gravity
                velocity.velocity += gravity * fixedDeltaTime;

                // Apply linear damping
                velocity.velocity *= linearDamping;

                // position
                pos.position += velocity.velocity * fixedDeltaTime;

                // angular motion
                angVel.zAngularVelocity *= angularDamping;
                rotation.rotation.z += angVel.zAngularVelocity * fixedDeltaTime;
                if (std::abs(velocity.velocity.x) < 0.1f && std::abs(velocity.velocity.y) < 0.2f) {
                    update.shouldBeUpdated = false;
                }
                if (update.shouldSpawnSmoke) {
                    spawnSmoke(ctx, pos.position);
                    update.spawnedSmoke = true;
                    update.shouldSpawnSmoke = false;
                }
            }
            };
    }
    void cigarettes::onDash(frameContext* fc, const entity entityPlayer, const bool facingRight) {
        RFCT_PROFILE_FUNCTION();
        if (ecs::get().valid(cigarettesVec[lastCigaretteIndex])) {
            fc->scene->deleteDynamicEntity(cigarettesVec[lastCigaretteIndex]);
        }
        entity newCigarette = constructCigarette(fc, entityPlayer, facingRight);
        cigarettesVec[lastCigaretteIndex] = (newCigarette);
        lastCigaretteIndex = (lastCigaretteIndex + 1) % cigarettesMaxCount;
    }
}

entity rfct::cigarettes::constructCigarette(const frameContext* fc, const entity entityPlayer, const bool facingRight) {
    RFCT_PROFILE_FUNCTION();
    constexpr float min_val = std::min(betweenVer, betweenHor);
    entt::registry& reg = ecs::get();

    glm::mat4 transMat = glm::translate(glm::mat4(1.f), glm::vec3(reg.get<positionComponent>(entityPlayer).position, 0.f));
    entity newCigarette = fc->scene->createDynamicRenderingEntity(&cigaretteModelVertices, &transMat);
    reg.emplace_or_replace<cigaretteUpdateComponent>(newCigarette, cigaretteUpdateComponent{ { true } });
    reg.emplace_or_replace<positionComponent>(newCigarette, positionComponent{ { reg.get<positionComponent>(entityPlayer).position } });
    reg.emplace_or_replace<velocityComponent>(newCigarette, velocityComponent{ { .5f * glm::vec2{facingRight ? -1.f : 1.f, 1.f} } });
    reg.emplace_or_replace<rotationComponent>(newCigarette, rotationComponent{});
    reg.emplace_or_replace<angularVelocityComponent>(newCigarette, angularVelocityComponent{ { randF() * 20.f + 20.f } });
    reg.emplace_or_replace<staticObjCollisionCallbackComponent>(newCigarette, staticObjCollisionCallbackComponent{ { onCollision_Cigarette_StaticObj } });
    reg.emplace_or_replace<gravityComponent>(newCigarette, gravityComponent{ 0.90f, false, 4.f });
    reg.emplace_or_replace<dynamicObjectTypeComponent>(newCigarette, dynamicObjectTypeComponent{ { dynamicObjectType::Cigarette } });
    reg.emplace_or_replace<dynamicBoxColliderComponent>(newCigarette, dynamicBoxColliderComponent{  { -min_val, -min_val }, { min_val, min_val } });
    reg.emplace_or_replace<rotationComponent>(newCigarette, rotationComponent{ {} });
    reg.emplace_or_replace<scaleComponent>(newCigarette, scaleComponent{ });

    return newCigarette;
}

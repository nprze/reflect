#include "cigarettes.h"
#include "world_p/components.h"
#include "context.h"
#include "world_p/scene.h"
#include "world_p/transform.h"
#include "world_p/ecs.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "world_p/object_components.h"
#include "world_p/decors/smokes.h"

namespace rfct {
    constexpr float angularDamping = 0.94f;
    constexpr glm::vec2 gravity{ 0.f, -5.f };
    constexpr float linearDamping = 0.9f;

    constexpr float betweenVer = 0.05f;
    constexpr float betweenHor = 0.11f;
    constexpr float restVer = .07f;
    constexpr float restHor = .13f;

    static flecs::query<cigaretteUpdateComponent, positionComponent, velocityComponent, angularVelocityComponent, rotationComponent> cigaretteComponentsQuery;
    static flecs::query<cigaretteUpdateComponent, dynamicSSBOIndexComponent, positionComponent, rotationComponent, scaleComponent> cigaretteMatricesQuery;
}

std::vector<rfct::Vertex> cigaretteVertices;
void rfct::initCigaretteVars(scene* scene)
{
    // vertices
    cigaretteVertices.resize(12);


    glm::vec3 white = { 0.6f, 0.6f, 0.6f };
    glm::vec3 black = { 0.f,0.f, 0.f };
    // background triangles
    glm::vec3 bg0 = glm::vec3(-restHor, restVer, 0);
    glm::vec3 bg1 = glm::vec3(restHor, restVer, 0);
    glm::vec3 bg2 = glm::vec3(-restHor, -restVer, 0);
    glm::vec3 bg3 = glm::vec3(restHor, -restVer, 0);


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
    glm::vec3 v0 = glm::vec3(-betweenHor, betweenVer, 0);
    glm::vec3 v1 = glm::vec3(betweenHor, betweenVer, 0);
    glm::vec3 v2 = glm::vec3(-betweenHor, -betweenVer, 0);
    glm::vec3 v3 = glm::vec3(betweenHor, -betweenVer, 0);


    cigaretteVertices[6].pos = v2;
    cigaretteVertices[7].pos = v1;
    cigaretteVertices[8].pos = v3;

    cigaretteVertices[9].pos = v0;
    cigaretteVertices[10].pos = v1;
    cigaretteVertices[11].pos = v2;


    for (uint8_t i = 6; i < 12; ++i) {
        cigaretteVertices[i].color = white;
    }


    // query

    cigaretteComponentsQuery =
        ecs::get().query_builder<cigaretteUpdateComponent, positionComponent, velocityComponent, angularVelocityComponent, rotationComponent>()
        .with(flecs::ChildOf, scene->sceneEntity)
        .build();

    cigaretteMatricesQuery =
        ecs::get().query_builder<cigaretteUpdateComponent, dynamicSSBOIndexComponent, positionComponent, rotationComponent, scaleComponent>()
        .with(flecs::ChildOf, scene->sceneEntity)
        .build();

}

void rfct::cleanupCigarettes()
{
    cigaretteComponentsQuery.~query();
    cigaretteMatricesQuery.~query();
}

void rfct::onCollision_Cigarette_StaticObj(entity cigarette, entity collidedWith, glm::vec2 resolution)
{
    auto* rot = cigarette.get_mut<rotationComponent>();
    auto* pos = cigarette.get_mut<positionComponent>();
    auto* vel = cigarette.get_mut<velocityComponent>();
    auto* angVel = cigarette.get_mut<angularVelocityComponent>();

    auto* collidedWithAABB = collidedWith.get<staticBoxColliderComponent>();
    

    pos->position += resolution;

    if (!cigarette.get<cigaretteUpdateComponent>()->spawnedSmoke) {
        cigarette.get_mut<cigaretteUpdateComponent>()->spawnedSmoke = true;
        cigarette.get_mut<cigaretteUpdateComponent>()->shouldSpawnSmoke = true;
    }
}

void rfct::updateCigarettes(frameContext* ctx) { // cigarette system
    sceneRenderData& rd = ctx->scene->getRenderData();
    cigaretteComponentsQuery.each([&](flecs::entity cigaretteEntity, cigaretteUpdateComponent& update, positionComponent& pos, velocityComponent& velocity, angularVelocityComponent& angVel, rotationComponent& rotation) {
        if (update.shouldBeUpdated) {
            for (uint8_t i = 0; i < ctx->fixedUpdateTimes; ++i) {
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

        }
        });
}

void rfct::updateCigarettesMatrixes(const frameContext* ctx)
{
    sceneRenderData& rd = ctx->scene->getRenderData();
    cigaretteMatricesQuery.each([&](flecs::entity cigaretteEntity, cigaretteUpdateComponent& upd, dynamicSSBOIndexComponent& ssboData, positionComponent& pos, rotationComponent& rot, scaleComponent& sc) {
        glm::mat4 mat = getModelMatrix(pos, rot, sc);
        rd.updateMat(ctx, ssboData.indexInSSBO, &mat);
    });
}

entity rfct::constructCigarette(const frameContext* fc, const entity entityPlayer, const bool facingRight)
{
    constexpr float min_val = std::min(betweenVer, betweenHor);

    glm::mat4 transMat = glm::translate(glm::mat4(1.f), glm::vec3(entityPlayer.get<positionComponent>()->position, 0.f));
    entity newCigarette = fc->scene->createDynamicRenderingEntity(&cigaretteVertices, &transMat);
    newCigarette.set<cigaretteUpdateComponent>({ true });
    newCigarette.set<positionComponent>({ entityPlayer.get<positionComponent>()->position });
    newCigarette.set<velocityComponent>({ .5f * glm::vec2{facingRight ? -1.f : 1.f, 1.f} });
    newCigarette.set<angularVelocityComponent>({ randF() * 20.f + 20.f });
    newCigarette.set<staticObjCollisionCallbackComponent>({ onCollision_Cigarette_StaticObj });
    newCigarette.set<gravityComponent>({ 0.90f, false, 4.f });
    newCigarette.set<dynamicObjectTypeComponent>({ dynamicObjectType::Cigarette });
    newCigarette.set<dynamicBoxColliderComponent>({ { -min_val, -min_val}, { min_val, min_val} });
    newCigarette.set<rotationComponent>({});
    newCigarette.set<scaleComponent>({});
    return newCigarette;
}

/*
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
*/
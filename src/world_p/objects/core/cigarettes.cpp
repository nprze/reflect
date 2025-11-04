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

    constexpr uint8_t cigarettesMaxCount = 4;

    std::vector<entity> cigarettesVec;
    uint8_t lastCigaretteIndex = 0;

    static flecs::query<cigaretteUpdateComponent, positionComponent, velocityComponent, angularVelocityComponent, rotationComponent> cigaretteComponentsQuery;
    static flecs::query<cigaretteUpdateComponent, dynamicSSBOIndexComponent, positionComponent, rotationComponent, scaleComponent> cigaretteMatricesQuery;


    std::vector<rfct::Vertex> cigaretteModelVertices;
    void cigarettes::initSystem()
    {
        cigarettesVec.assign(cigarettesMaxCount, entity::null());

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
    void cigarettes::createQueries()
    {
        cigaretteComponentsQuery =
            ecs::get().query_builder<cigaretteUpdateComponent, positionComponent, velocityComponent, angularVelocityComponent, rotationComponent>()
            .build();

        cigaretteMatricesQuery =
            ecs::get().query_builder<cigaretteUpdateComponent, dynamicSSBOIndexComponent, positionComponent, rotationComponent, scaleComponent>()
            .build();
    }
    void cigarettes::deleteQueries()
    {
        cigaretteComponentsQuery.~query();
        cigaretteMatricesQuery.~query();
    }
    void cigarettes::spawnData(scene* s, sceneSerializedData* sd)
    {
    }
    void cigarettes::resetLevel(const frameContext* ctx)
    {
        for (entity& e : cigarettesVec)
        {
            if (e!= entity::null())
                ctx->scene->deleteDynamicEntity(e);
            e = entity::null();
        }
    }
    void cigarettes::updateVisuals(const frameContext* ctx) {
        sceneRenderData& rd = ctx->scene->getRenderData();
        cigaretteMatricesQuery.each([&](flecs::entity cigaretteEntity, cigaretteUpdateComponent& upd, dynamicSSBOIndexComponent& ssboData, positionComponent& pos, rotationComponent& rot, scaleComponent& sc) {
            glm::mat4 mat = getModelMatrix(pos, rot, sc);
            rd.updateMat(ctx, ssboData.indexInSSBO, &mat);
            });

    }
    void cigarettes::updateSystem(frameContext* ctx)
    {
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
    void cigarettes::onDash(frameContext* fc, const entity entityPlayer, const bool facingRight)
    {
        entity newCigarette = constructCigarette(fc, entityPlayer, facingRight);
        if (cigarettesVec[lastCigaretteIndex] != entity::null()) {
            fc->scene->deleteDynamicEntity(cigarettesVec[lastCigaretteIndex]);
        }
        cigarettesVec[lastCigaretteIndex] = (newCigarette);
        lastCigaretteIndex = (lastCigaretteIndex + 1) % cigarettesMaxCount;
    }
}

namespace rfct {
    void onCollision_Cigarette_StaticObj(entity cigarette, entity collidedWith, glm::vec2 resolution)
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
}


entity rfct::cigarettes::constructCigarette(const frameContext* fc, const entity entityPlayer, const bool facingRight)
{
    constexpr float min_val = std::min(betweenVer, betweenHor);

    glm::mat4 transMat = glm::translate(glm::mat4(1.f), glm::vec3(entityPlayer.get<positionComponent>()->position, 0.f));
    entity newCigarette = fc->scene->createDynamicRenderingEntity(&cigaretteModelVertices, &transMat);
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

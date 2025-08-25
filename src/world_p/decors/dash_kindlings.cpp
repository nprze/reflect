#include "dash_kindlings.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include <glm/ext.hpp>
#include "world_p/scene.h"
#include "world_p/ecs.h"
#include "world_p/transform.h"

namespace rfct {
    constexpr float inside = 0.07f;
    constexpr float outside = 0.09f;


    constexpr float angularDamping = 0.94f;

    static flecs::query<kindlingParticleComponent, sinusoidFloatComponent, angularVelocityComponent, positionComponent, rotationComponent, scaleComponent> kindlingParticlesComponentsQuery;
    static flecs::query<kindlingParticleComponent, dynamicSSBOIndexComponent, positionComponent, rotationComponent, scaleComponent> kindlingParticlesQuery;
}

std::vector<rfct::Vertex> kindlingVerticesRed;
std::vector<rfct::Vertex> kindlingVerticesOrange;
void rfct::initKindlingsVars(scene* parentScene)
{
    RFCT_PROFILE_FUNCTION();

    kindlingVerticesRed.clear();
    kindlingVerticesOrange.clear();
    kindlingVerticesRed.resize(24);
    kindlingVerticesOrange.resize(24);

    glm::vec3 red = { 0.8f, 0.2f, 0.2f };
    glm::vec3 black = { 0.f, 0.f, 0.f };

    glm::vec3 center = { 0, 0, 0 };

    float r_outer = outside;
    std::vector<glm::vec3> hexOuter(6);
    for (int i = 0; i < 6; i++) {
        float angle = glm::radians(60.0f * i + 30.0f);
        hexOuter[i] = glm::vec3(r_outer * cos(angle), r_outer * sin(angle), 0);
    }

    int v = 0;

    kindlingVerticesRed[v + 0].pos = hexOuter[0];
    kindlingVerticesRed[v + 1].pos = hexOuter[2];
    kindlingVerticesRed[v + 2].pos = hexOuter[4];
    kindlingVerticesRed[v + 0].color = black;
    kindlingVerticesRed[v + 1].color = black;
    kindlingVerticesRed[v + 2].color = black;
    v += 3;

    int triOuter[3][3] = { {0,1,2}, {2,3,4}, {4,5,0} };
    for (int t = 0; t < 3; t++) {
        for (int j = 0; j < 3; j++) {
            kindlingVerticesRed[v + j].pos = hexOuter[triOuter[t][j]];
            kindlingVerticesRed[v + j].color = black;
        }
        v += 3;
    }

    float r_inner = inside;
    std::vector<glm::vec3> hexInner(6);
    for (int i = 0; i < 6; i++) {
        float angle = glm::radians(60.0f * i + 30.0f);
        hexInner[i] = glm::vec3(r_inner * cos(angle), r_inner * sin(angle), 0);
    }

    kindlingVerticesRed[v + 0].pos = hexInner[0];
    kindlingVerticesRed[v + 1].pos = hexInner[2];
    kindlingVerticesRed[v + 2].pos = hexInner[4];
    kindlingVerticesRed[v + 0].color = red;
    kindlingVerticesRed[v + 1].color = red;
    kindlingVerticesRed[v + 2].color = red;
    v += 3;
    
    for (int t = 0; t < 3; t++) {
        for (int j = 0; j < 3; j++) {
            kindlingVerticesRed[v + j].pos = hexInner[triOuter[t][j]];
            kindlingVerticesRed[v + j].color = red;
        }
        v += 3;
    }




    kindlingParticlesComponentsQuery =
        ecs::get().query_builder<kindlingParticleComponent, sinusoidFloatComponent, angularVelocityComponent, positionComponent, rotationComponent, scaleComponent>()
        .with(flecs::ChildOf, parentScene->sceneEntity)
        .build();
    kindlingParticlesQuery =
        ecs::get().query_builder<kindlingParticleComponent, dynamicSSBOIndexComponent, positionComponent, rotationComponent, scaleComponent>()
        .with(flecs::ChildOf, parentScene->sceneEntity)
        .build();
}

void rfct::cleanupKindlings()
{
    kindlingParticlesComponentsQuery.~query();
    kindlingParticlesQuery.~query();
}

void rfct::spawnKindling(frameContext* fc, const glm::vec2& position, const glm::vec2& playerVel , uint32_t var)
{
    glm::mat4 transMat = glm::translate(glm::mat4(1.f), glm::vec3(position, 0.f));
    entity kindling = fc->scene->createDynamicRenderingEntity(&kindlingVerticesRed, &transMat);
    glm::vec2 dir = glm::normalize(-playerVel);
    kindling.set<kindlingParticleComponent>({ dir, 0.5f, 0.f });
    kindling.set<sinusoidFloatComponent>({
            glm::linearRand(0.5f, 2.0f),
            glm::linearRand(0.5f, 3.0f),
            glm::linearRand(0.f, 6.28f)
        });

    kindling.set<angularVelocityComponent>({ glm::linearRand(-60.f, 60.f) });

    kindling.set<positionComponent>({ position });
    kindling.set<rotationComponent>({});
    kindling.set<scaleComponent>({ {1, 1} });

    kindling.set<dynamicObjectTypeComponent>({ dynamicObjectType::Kindling });
}

void rfct::updateKindlings(frameContext* ctx)
{
    std::vector<entity> toBeRemovedEnt;
    for (uint8_t i = 0; i < ctx->fixedUpdateTimes; i++) {
        kindlingParticlesComponentsQuery.each([&](flecs::entity smokeParticle, kindlingParticleComponent& particleData, sinusoidFloatComponent& sinFloat, angularVelocityComponent& angVel, positionComponent& pos, rotationComponent& rot, scaleComponent& sc) {
            if (particleData.currentProgress < particleData.fullLenght) {
                particleData.currentProgress += fixedDeltaTime;

                float progressPercentage = particleData.currentProgress / particleData.fullLenght;
                float curScale = -std::pow(progressPercentage, 4) + 1;
                curScale = std::max(curScale, 0.f);
                sc.scale = { curScale ,curScale };

                // Forward motion
                pos.position += particleData.direction * fixedDeltaTime;

                // Sinusoidal float
                glm::vec2 perp = glm::normalize(glm::vec2(-particleData.direction.y, particleData.direction.x));
                float t = particleData.currentProgress;
                float offset = sinFloat.amplitude * sin(sinFloat.frequency * t + sinFloat.phase);
                pos.position += perp * offset * fixedDeltaTime;

                // Rotation damping
                angVel.zAngularVelocity *= angularDamping;
                rot.rotation.z += angVel.zAngularVelocity * fixedDeltaTime;
            }
            else {
                toBeRemovedEnt.push_back(smokeParticle);
            }
            });
    }
    for (entity e : toBeRemovedEnt) {
        if (ecs::get().is_valid(e)) {
            ctx->scene->deleteDynamicEntity(e);
        }
    }
}

void rfct::updateKindlingMatrices(frameContext* ctx)
{
    sceneRenderData& rd = ctx->scene->getRenderData();
    kindlingParticlesQuery.each([&](flecs::entity smokeParticle, kindlingParticleComponent& dis, dynamicSSBOIndexComponent& ssboData, positionComponent& pos, rotationComponent& rot, scaleComponent& sc) {
            glm::mat4 mat = getModelMatrix(pos, rot, sc);
            rd.updateMat(ctx, ssboData.indexInSSBO, &mat);
        });
}

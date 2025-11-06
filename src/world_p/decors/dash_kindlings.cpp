#include "dash_kindlings.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include <glm/ext.hpp>
#include "world_p/scene.h"
#include "world_p/ecs.h"
#include "world_p/transform.h"

namespace rfct {
    constexpr float inside = 0.08f;
    constexpr float outside = 0.1f;


    constexpr float angularDamping = 0.94f;
}

std::vector<rfct::Vertex> kindlingVertices [3];
void rfct::initKindlingsVars(scene* parentScene)
{
    RFCT_PROFILE_FUNCTION();

    for (uint8_t i = 0; i < 3; ++i) {
        kindlingVertices[i].clear();
        kindlingVertices[i].resize(6);
    }

    glm::vec3 red = { 0.8f, 0.2f, 0.2f };
    glm::vec3 orange = { 0.7f, 0.3f, 0.2f };
    glm::vec3 yellow = { 0.6f, 0.4f, 0.2f };
    glm::vec3 black = { 0.f, 0.f, 0.f };

    glm::vec3 center = { 0, 0, 0 };

    float r_outer = outside;
    std::vector<glm::vec3> hexOuter(3);
    for (int i = 0; i < 3; i++) {
        float angle = glm::radians(120.0f * i);
        hexOuter[i] = glm::vec3(r_outer * cos(angle), r_outer * sin(angle), 0);
    }

    for (uint8_t i = 0; i < 3; ++i) {
        kindlingVertices[i][0].pos = hexOuter[0];
        kindlingVertices[i][1].pos = hexOuter[1];
        kindlingVertices[i][2].pos = hexOuter[2];
        kindlingVertices[i][0].color = black;
        kindlingVertices[i][1].color = black;
        kindlingVertices[i][2].color = black;
    }

    float r_inner = inside;
    std::vector<glm::vec3> hexInner(3);
    for (int i = 0; i < 3; i++) {
        float angle = glm::radians(120.f * i);
        hexInner[i] = glm::vec3(r_inner * cos(angle), r_inner * sin(angle), 0);
    }
    for (uint8_t i = 0; i < 3; ++i) {
        kindlingVertices[i][3 + 0].pos = hexInner[0];
        kindlingVertices[i][3 + 1].pos = hexInner[1];
        kindlingVertices[i][3 + 2].pos = hexInner[2];
        glm::vec3 color;
        if (i == 0) color = red;
        if (i == 1) color = orange;
        if (i == 2) color = yellow;
        kindlingVertices[i][3 + 0].color = color;
        kindlingVertices[i][3 + 1].color = color;
        kindlingVertices[i][3 + 2].color = color;
    }
}

void rfct::cleanupKindlings()
{
}

void rfct::spawnKindling(frameContext* fc, const glm::vec2& position, const glm::vec2& playerVel , uint32_t var)
{
    for (uint32_t i = 0; i < var + 1; ++i) {
        glm::mat4 transMat = glm::translate(glm::mat4(1.f), glm::vec3(position, 0.f));
        entity kindling = fc->scene->createDynamicRenderingEntity(&kindlingVertices[2 - var], &transMat);
        glm::vec2 per = glm::normalize(glm::cross(glm::vec3(-playerVel, 0.f), glm::vec3(0, 0, 1)));
        glm::vec2 dir = glm::normalize(-playerVel + (per * glm::linearRand(-0.3f, 0.3f)));

        entt::registry& reg = ecs::get();

        reg.emplace<kindlingParticleComponent>(kindling, kindlingParticleComponent{ dir, 0.5f, 0.f});
        reg.emplace<sinusoidFloatComponent>(kindling, sinusoidFloatComponent{ glm::linearRand(0.5f, 2.0f), glm::linearRand(0.5f, 3.0f), glm::linearRand(0.f, 6.28f) });
        reg.emplace<angularVelocityComponent>(kindling, angularVelocityComponent{ {glm::linearRand(-10.f, 10.f)} });
        reg.emplace_or_replace<positionComponent>(kindling, positionComponent{ {position} });
        reg.emplace_or_replace<rotationComponent>(kindling, rotationComponent{ {} });
        reg.emplace<scaleComponent>(kindling, scaleComponent{ {0, 0} });
        reg.emplace<dynamicObjectTypeComponent>(kindling, dynamicObjectTypeComponent{ {dynamicObjectType::Kindling} });

    }
}

void rfct::updateKindlings(frameContext* ctx)
{
    for (uint8_t i = 0; i < ctx->fixedUpdateTimes; i++) {
        auto kindlingParticlesComponentsQuery = ecs::get().view<kindlingParticleComponent, sinusoidFloatComponent, angularVelocityComponent, positionComponent, rotationComponent, scaleComponent, dynamicSSBOIndexComponent>();
        for (auto [smokeParticle, particleData, sinFloat, angVel, pos, rot, sc, ssbo] : kindlingParticlesComponentsQuery.each()) {
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
                ctx->scene->getRenderData().removeDynamicEntity(smokeParticle);
                ecs::get().destroy(smokeParticle);
            }
            };
    }
}

void rfct::updateKindlingMatrices(frameContext* ctx)
{
    sceneRenderData& rd = ctx->scene->getRenderData();
    auto kindlingParticlesComponentsQuery = ecs::get().view<kindlingParticleComponent, dynamicSSBOIndexComponent, positionComponent, rotationComponent, scaleComponent>();
    for (auto [smokeParticle, dis, ssboData, pos, rot, sc] : kindlingParticlesComponentsQuery.each()) {
        glm::mat4 mat = getModelMatrix(pos, rot, sc);
        rd.updateMat(ctx, ssboData.indexInSSBO, &mat);
    };
}

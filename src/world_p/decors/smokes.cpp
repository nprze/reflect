#include "smokes.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include <glm/ext.hpp>
#include "world_p/scene.h"
#include "world_p/ecs.h"
#include "world_p/transform.h"

namespace rfct {
    constexpr float inside = 0.3f;
    constexpr float outside = 0.34f;


    constexpr float angularDamping = 0.94f;
}

std::vector<rfct::Vertex> smokeVertices;
void rfct::initSmokeVars(scene* parentScene)
{
    RFCT_PROFILE_FUNCTION();
    smokeVertices.clear();
    smokeVertices.resize(24);

    glm::vec3 white = { 0.3f, 0.3f, 0.3f };
    glm::vec3 black = { 0.f, 0.f, 0.f };

    glm::vec3 center = { 0, 0, 0 };

    float r_outer = outside;
    std::vector<glm::vec3> hexOuter(6);
    for (int i = 0; i < 6; i++) {
        float angle = glm::radians(60.0f * i + 30.0f);
        hexOuter[i] = glm::vec3(r_outer * cos(angle), r_outer * sin(angle), 0);
    }

    int v = 0;

    smokeVertices[v + 0].pos = hexOuter[0];
    smokeVertices[v + 1].pos = hexOuter[2];
    smokeVertices[v + 2].pos = hexOuter[4];
    smokeVertices[v + 0].color = black;
    smokeVertices[v + 1].color = black;
    smokeVertices[v + 2].color = black;
    v += 3;

    int triOuter[3][3] = { {0,1,2}, {2,3,4}, {4,5,0} };
    for (int t = 0; t < 3; t++) {
        for (int j = 0; j < 3; j++) {
            smokeVertices[v + j].pos = hexOuter[triOuter[t][j]];
            smokeVertices[v + j].color = black;
        }
        v += 3;
    }

    float r_inner = inside;
    std::vector<glm::vec3> hexInner(6);
    for (int i = 0; i < 6; i++) {
        float angle = glm::radians(60.0f * i + 30.0f);
        hexInner[i] = glm::vec3(r_inner * cos(angle), r_inner * sin(angle), 0);
    }

    int triInner[4][3] = { {0,1,2}, {0,2,3}, {3,4,5}, {3,5,0} };

    float f = 0.95f; 

    for (int t = 0; t < 4; t++) {
        glm::vec3 centroid(0.0f);
        for (int j = 0; j < 3; j++) {
            centroid += hexInner[triInner[t][j]];
        }
        centroid /= 3.0f;

        for (int j = 0; j < 3; j++) {
            glm::vec3 orig = hexInner[triInner[t][j]];
            glm::vec3 inset = centroid + f * (orig - centroid);
            smokeVertices[v + j].pos = inset;
            smokeVertices[v + j].color = white;
        }
        v += 3;
    }
}

void rfct::cleanupSmokes()
{
}

void rfct::spawnSmoke(frameContext* fc, const glm::vec2& position, const glm::vec2& direction, uint32_t particleCount, float lifetimeSec)
{
    for (uint32_t i = 0; i < particleCount; ++i) {
        glm::vec2 randomOffset = glm::linearRand(0.001f * direction,  0.3f * direction);
        glm::vec2 spawnPos = position + randomOffset;

        glm::mat4 transMat = glm::translate(glm::mat4(1.f), glm::vec3(spawnPos, 0.f));
        float colorIntennsities[4];
        for (uint32_t i = 0; i < 4; i++) {
            colorIntennsities[i] = randF()* 0.2 +0.3;
        }
        setColors(colorIntennsities);
        entity smoke = fc->scene->createDynamicRenderingEntity(&smokeVertices, &transMat);

        entt::registry& reg = ecs::get();

        reg.emplace<smokeFinishedComponent>(smoke, smokeFinishedComponent{ {false} });
        glm::vec2 randomDir = glm::normalize(glm::linearRand(direction, glm::vec2(-direction.x, direction.y))) * (0.8f + randF() * 0.4f);
        reg.emplace<smokeParticleComponent>(smoke, smokeParticleComponent{ {randomDir} });
        reg.emplace<smokeDisperseComponent>(smoke, smokeDisperseComponent{(float)(glm::linearRand(0.5, 1.5)), 0.f});
        reg.emplace_or_replace<positionComponent>(smoke, positionComponent{ {spawnPos} });
        reg.emplace<sinusoidFloatComponent>(smoke, sinusoidFloatComponent{ glm::linearRand(0.5f, 2.0f), glm::linearRand(0.5f, 3.0f), glm::linearRand(0.f, 6.28f) });
        reg.emplace<angularVelocityComponent>(smoke, angularVelocityComponent{ {glm::linearRand(-60.f, 60.f)} });
        reg.emplace_or_replace<rotationComponent>(smoke, rotationComponent{ {} });
        reg.emplace_or_replace<scaleComponent>(smoke, scaleComponent{ {0, 0} });
        reg.emplace<dynamicObjectTypeComponent>(smoke, dynamicObjectTypeComponent{ {dynamicObjectType::Smoke} });

    }
}

void rfct::setColors(float* is)
{
    uint32_t s = 4 * 3;
    for (int j = 0; j < 3; j++) {
        smokeVertices[s + j].color = { *is, *is, *is };
    }
    is++;
    s += 3;
    for (int j = 0; j < 3; j++) {
        smokeVertices[s + j].color = { *is, *is, *is };
    }
    is++;
    s += 3;
    for (int j = 0; j < 3; j++) {
        smokeVertices[s + j].color = { *is, *is, *is };
    }
    is++;
    s += 3;
    for (int j = 0; j < 3; j++) {
        smokeVertices[s + j].color = { *is, *is, *is };
    }
    is++;
    s += 3;
}

void rfct::updateSmokes(frameContext* ctx)
{
	// fixed update
    auto smokeParticlesComponentsQuery = ecs::get().view<smokeParticleComponent, smokeDisperseComponent, sinusoidFloatComponent, angularVelocityComponent, positionComponent, rotationComponent, scaleComponent, dynamicSSBOIndexComponent>();
    for (auto [smokeParticle, particleDir, disperse, smokeFloat, angVel, pos, rot, sc, ssbo] : smokeParticlesComponentsQuery.each()) {
        if (disperse.currentProgress < disperse.fullLenght) {
            disperse.currentProgress += fixedDeltaTime;
            float progressPercentage = disperse.currentProgress / disperse.fullLenght;
            float curScale = -std::pow(2.f* std::pow(progressPercentage, 1.5f) - 1, 4) + 1;
            curScale = std::max(curScale, 0.f);
            sc.scale = { curScale ,curScale };

            // Forward motion
            pos.position += particleDir.direction * fixedDeltaTime;

            // Sinusoidal float
            glm::vec2 perp = glm::normalize(glm::vec2(-particleDir.direction.y, particleDir.direction.x));
            float t = disperse.currentProgress;
            float offset = smokeFloat.amplitude * sin(smokeFloat.frequency * t + smokeFloat.phase);
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

void rfct::updateSmokeMatrices(frameContext* ctx)
{
    renderData& rd = ctx->scene->getRenderData();
    auto smokeParticlesQuery = ecs::get().view<smokeDisperseComponent, dynamicSSBOIndexComponent, positionComponent, rotationComponent, scaleComponent, dynamicSSBOIndexComponent>();
    for (auto [smokeParticle, dis, ssboData, pos, rot, sc,ssbo] : smokeParticlesQuery.each()) {
        glm::mat4 mat = getModelMatrix(pos, rot, sc);
        rd.updateMat(ctx, ssboData.indexInSSBO, &mat);
    };
}

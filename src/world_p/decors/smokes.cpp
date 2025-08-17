#include "smokes.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace rfct {
    constexpr float betweenVer = 0.5f;
    constexpr float betweenHor = 0.5f;
    constexpr float restVer = .55f;
    constexpr float restHor = .55f;
    static flecs::query<smokeParticleComponent, positionComponent, velocityComponent, angularVelocityComponent, rotationComponent> cigaretteComponentsQuery;
}

std::vector<rfct::Vertex> smokeVertices;
void rfct::initSmokeVars()
{

    // vertices
    smokeVertices.resize(12);


    glm::vec3 white = { 0.6f, 0.6f, 0.6f };
    glm::vec3 black = { 0.f,0.f, 0.f };
    // background triangles
    glm::vec3 bg0 = glm::vec3(-restHor, restVer, 0);
    glm::vec3 bg1 = glm::vec3(restHor, restVer, 0);
    glm::vec3 bg2 = glm::vec3(-restHor, -restVer, 0);
    glm::vec3 bg3 = glm::vec3(restHor, -restVer, 0);


    smokeVertices[0].pos = bg0;
    smokeVertices[1].pos = bg1;
    smokeVertices[2].pos = bg2;

    smokeVertices[3].pos = bg3;
    smokeVertices[4].pos = bg1;
    smokeVertices[5].pos = bg2;

    for (uint8_t i = 0; i < 6; ++i) {
        smokeVertices[i].color = black;
    }


    // color triangles
    glm::vec3 v0 = glm::vec3(-betweenHor, betweenVer, 0);
    glm::vec3 v1 = glm::vec3(betweenHor, betweenVer, 0);
    glm::vec3 v2 = glm::vec3(-betweenHor, -betweenVer, 0);
    glm::vec3 v3 = glm::vec3(betweenHor, -betweenVer, 0);


    smokeVertices[6].pos = v2;
    smokeVertices[7].pos = v1;
    smokeVertices[8].pos = v3;

    smokeVertices[9].pos = v0;
    smokeVertices[10].pos = v1;
    smokeVertices[11].pos = v2;


    for (uint8_t i = 6; i < 12; ++i) {
        smokeVertices[i].color = white;
    }

}


entity rfct::spawnSmoke(frameContext* fc, const glm::vec2& position, const glm::vec2& direction, uint32_t particleCount, float lifetimeSec)
{/*
    glm::mat4 transMat = glm::translate(glm::mat4(1.f), glm::vec3(entityPlayer.get<positionComponent>()->position, 0.f));
    entity smoke = fc->scene->createDynamicRenderingEntity(&smokeVertices, &transMat);
    smoke.set<cigaretteUpdateComponent>({ true });
    smoke.set<positionComponent>({ entityPlayer.get<positionComponent>()->position });
    smoke.set<velocityComponent>({ .5f * glm::vec2{facingRight ? -1.f : 1.f, 1.f} });
    smoke.set<angularVelocityComponent>({ randF() * 20.f + 20.f });
    smoke.set<rotationComponent>({});
    smoke.set<staticObjCollisionCallbackComponent>({ onCollision_Cigarette_StaticObj });
    smoke.set<scaleComponent>({});
    smoke.set<gravityComponent>({ 0.97f, false, 5.f });
    smoke.set<dynamicObjectTypeComponent>({ dynamicObjectType::Cigarette });
    smoke.set<dynamicBoxColliderComponent>({ { -min_val, -min_val}, { min_val, min_val} });
    return smoke;
    */
    return entity();
}

void rfct::updateSmokes(frameContext* ctx)
{
    for (uint8_t i = 0; i < ctx->fixedUpdateTimes; i++) {

    }
}

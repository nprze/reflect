#include "cigarettes.h"
#include "world_p/components.h"
#include "context.h"
#include "world_p/scene.h"
#include "world_p/transform.h"

void rfct::onCollision_Cigarette_StaticObj(entity cigarette, entity collidedWith, glm::vec2 resolution)
{
    velocityComponent* vel = cigarette.get_mut<velocityComponent>();
    if (resolution.y!=0)
        vel->velocity.y = -vel->velocity.y * 0.3f;

    positionComponent* pos = cigarette.get_mut<positionComponent>();
    pos->position += resolution;
}

void rfct::updateCigarette(entity cigaretteEntity, const frameContext* fc) {
    auto* pos = cigaretteEntity.get_mut<positionComponent>();
    auto* velocity = cigaretteEntity.get_mut<velocityComponent>();

    constexpr glm::vec2 gravity{ 0.f, -5.f };

    for (uint8_t i = 0; i < fc->fixedUpdateTimes; ++i) {
        velocity->velocity += gravity * fixedDeltaTime;
        velocity->velocity *= 0.9f;
        pos->position += velocity->velocity * fixedDeltaTime;
    }
    glm::mat4 mat = getModelMatrixFromEntity(cigaretteEntity);
    fc->scene->getRenderData().updateMat(fc, cigaretteEntity.get<dynamicSSBOIndexComponent>()->indexInSSBO, &mat);

}

#include "enemy.h"
#include "world_p/ecs.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "world_p/transform.h"
#include "world_p/scene.h"
#include "world_p/physics/collision.h"
#include "world_p/world.h"
#include "renderer_p/frame_anim/anim_buffer.h"
#include "assets/assets_manager.h"
#include "world_p/objects/objects.h"

namespace rfct {
	constexpr float maxEnemySpeed = 0.2f;
	constexpr float oneTurnTime = .5f;

	animationBuffer animBuffer;

	static flecs::query<velocityComponent, positionComponent, scaleComponent, enemyComponent> enemyQuery;
	void onCollision_Enemy_StaticObj(entity enemy, entity collidedWith, glm::vec2 resolution) {
		positionComponent* pos = enemy.get_mut<positionComponent>();
		pos->position += resolution;
		velocityComponent* vel = enemy.get_mut<velocityComponent>();
		if (resolution.x != 0) {
			vel->velocity.x = 0.f;
			enemyComponent* enComp = enemy.get_mut<enemyComponent>();
			if (enComp->turningTime == 0.f)
				enComp->turningTime = oneTurnTime;
		}
		else if (resolution.y != 0) {
			vel->velocity.y = 0.f;

		}
	}
	void onCollision_Enemy_DynamicObj(entity enemy, entity collidedWith) {
		if (collidedWith.get<dynamicObjectTypeComponent>()->passable) return;
		const dynamicBoxColliderComponent* playerCol = collidedWith.get<dynamicBoxColliderComponent>();
		const positionComponent* playerPos = collidedWith.get<positionComponent>();
		const dynamicBoxColliderComponent* enemyCol = enemy.get<dynamicBoxColliderComponent>();
		positionComponent* enemyPos = enemy.get_mut<positionComponent>();
		glm::vec2 resolution = ResolveAABBCollision(enemyPos->position + enemyCol->min, enemyPos->position + enemyCol->max, playerCol->min + playerPos->position, playerCol->max + playerPos->position);
		enemyPos->position += resolution;
		velocityComponent* vel = enemy.get_mut<velocityComponent>();
		if (resolution.y == 0) {
			vel->velocity.x = 0.f;
			enemyComponent* enComp = enemy.get_mut<enemyComponent>();
			if (enComp->turningTime == 0.f)
				enComp->turningTime = oneTurnTime;
		}
		else if (resolution.x == 0) {
			vel->velocity.y = 0.f;

		}
		if (collidedWith.get<dynamicObjectTypeComponent>()->type == dynamicObjectType::Player) {
			if (!(resolution.x == 0 && resolution.y == 0)) {
				collidedWith.get_mut<playerLifeComponent>()->alive = false;
			}
		}
	}
};

namespace rfct {
	void enemies::initSystem()
	{
		animBuffer.init(10000);
	}

	void enemies::createQueries()
	{
		enemyQuery =
			ecs::get().query_builder<velocityComponent, positionComponent, scaleComponent, enemyComponent>()
			.build();
	}
	void enemies::deleteQueries() {
		enemyQuery.~query();
	}

	void enemies::spawnData(scene* s, sceneSerializedData* sd) {

		for (const BasicEnemyInfo& e : sd->enemies) {

			dynamicBoxColliderComponent bounds = {};
			bounds.min = e.min;
			bounds.max = e.max;

			const float oneSeventytieth = 1.f / 70.f;
			transform trans = {};
			trans.pos = { e.position };
			trans.scale.scale = { oneSeventytieth , oneSeventytieth };
			glm::mat4 model = getModelMatrixFromTransform(trans);
			frameContext noCtx{};

			staticObjCollisionCallbackComponent colCallback;
			colCallback.handler = onCollision_Enemy_StaticObj;
			dynamicObjCollisionCallbackComponent dynColCallback;
			dynColCallback.handler = onCollision_Enemy_DynamicObj;

			enemyComponent eComp = {};

			std::string singleAnimName;
			std::istringstream iss(e.animation);

			uint32_t ssboMatrixIndex = s->getRenderData().addDynamicMat(&noCtx, &model);

			iss >> singleAnimName;
			eComp.walkFrameAnim = AssetsManager::get().loadAnimation(singleAnimName + ".txt", &animBuffer, ssboMatrixIndex);

			iss >> singleAnimName;
			eComp.turnFrameAnim = AssetsManager::get().loadAnimation(singleAnimName + ".txt", &animBuffer, ssboMatrixIndex);

			iss >> singleAnimName;
			eComp.dieFrameAnim = AssetsManager::get().loadAnimation(singleAnimName + ".txt", &animBuffer, ssboMatrixIndex);

			eComp.animIndex = 0;
			eComp.frameIndex = 0;

			entity enemy = ecs::get().entity<>()
				.set<dynamicSSBOIndexComponent>({ ssboMatrixIndex })
				.set<rotationComponent>({ trans.rot })
				.set<scaleComponent>({ trans.scale })
				.set<positionComponent>({ trans.pos })
				.set<gravityComponent>({ 0.97, true, 3.f })
				.set<velocityComponent>({ glm::vec2(0.f,0.f) })
				.set<dynamicBoxColliderComponent>(bounds)
				.set<staticObjCollisionCallbackComponent>(colCallback)
				.set<dynamicObjCollisionCallbackComponent>(dynColCallback)
				.set<enemyComponent>(eComp)
				.set<dynamicObjectTypeComponent>({ dynamicObjectType::Enemy, false });

		}
	};

	void enemies::resetLevel(const frameContext* ctx) {};

	void enemies::updateVisuals(const frameContext* ctx) {
		enemyQuery.each([&](flecs::entity e, velocityComponent& vel, positionComponent& pos, scaleComponent& sc, enemyComponent& en) {
			ctx->scene->updateTransformData(ctx, e);
			});
	};

	void enemies::updateSystem(frameContext* ctx) {

		enemyQuery.each([&](flecs::entity e, velocityComponent& vel, positionComponent& pos, scaleComponent& sc, enemyComponent& en) {

			frameAnimation* currentAnim = (frameAnimation*)((char*)&en.walkFrameAnim + (en.animIndex * sizeof(frameAnimation)));
			en.turningTime = std::clamp(en.turningTime - ctx->dt, 0.f, oneTurnTime);
			if (en.turningTime != 0.f) {
				if (en.animIndex != 1) {
					// change anim to turn
					en.animIndex = 1;
					en.frameIndex = 0;
					en.bufferOffset = 0;
				}
			}
			else {
				if (en.animIndex == 1) {
					// switch sides XD
					en.facingRight = !en.facingRight;

					// change anim to walk
					en.animIndex = 0;
					en.frameIndex = 0;
					en.bufferOffset = 0;
				}
				else {
				}
			}

			en.timeSinceFrameChanged += ctx->dt;
			if (en.timeSinceFrameChanged > currentAnim->timePerFrame) {
				if (!currentAnim->shouldBeRepeated && en.frameIndex == currentAnim->frameCount - 1) {
					currentAnim->endedPlaying = true;
				}
				else {
					// loop
					en.timeSinceFrameChanged = std::fmod(en.timeSinceFrameChanged, currentAnim->timePerFrame);

					en.bufferOffset += currentAnim->trianglesPerFrame[en.frameIndex] * 3 * sizeof(Vertex);
					en.frameIndex += 1;
					if (en.frameIndex > currentAnim->frameCount - 1) {
						en.frameIndex = 0;
						en.bufferOffset = 0;
					}
				}
			}

			sc.scale.x = std::abs(sc.scale.x) * (en.facingRight ? 1.f : -1.f);
			glm::vec2 rayMin = pos.position + glm::vec2{ 0.3f * (en.facingRight ? 1.f : -1.f), 0.f };
			glm::vec2 rayMax = pos.position + glm::vec2{ 0.3f * (en.facingRight ? 1.f : -1.f), -1.f };
			bool groundBefore = checkRayStatic(StaticObjsBVHnodes.back(), rayMin, rayMax);
			if (en.turningTime != 0.f) {
				vel.velocity.x = 0.f;
			}
			else {
				vel.velocity.x = std::clamp(en.facingRight ? 1.f : -1.f * maxEnemySpeed, -maxEnemySpeed, maxEnemySpeed);
			}
			if (!groundBefore) {
				if (en.turningTime == 0.f)
					en.turningTime = oneTurnTime;
			}
			}
		);
	};

	void enemies::cleanupSystem() {
		animBuffer.cleanup();
		deleteQueries();
	}

	void enemies::drawFrameAnimSprites(vk::CommandBuffer& cmd, frameContext* ctx)
	{
		vk::Buffer vertexBuffers[] = { animBuffer.getBuffer() };
		enemyQuery.each([&](flecs::entity e, velocityComponent& vel, positionComponent& pos, scaleComponent& sc, enemyComponent& en) {

			frameAnimation* currentAnim = (frameAnimation*)((char*)&en.walkFrameAnim + (en.animIndex * sizeof(frameAnimation)));
			vk::DeviceSize offsets[] = { currentAnim->bufferOffsetInBytes + en.bufferOffset };
			cmd.bindVertexBuffers(0, 1, vertexBuffers, offsets);
			cmd.draw(currentAnim->trianglesPerFrame[en.animIndex] * 3, 1, 0, 0);

			});
	}
}
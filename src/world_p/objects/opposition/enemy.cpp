#include "enemy.h"
#include "assets/mesh_load.h"
#include "renderer_p/frame_anim/anim_buffer.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "world_p/transform.h"
#include "world_p/scene.h"
#include "world_p/physics/collision.h"
#include "world_p/world.h"
#include "world_p/objects/objects.h"

constexpr float maxEnemySpeed = 0.2f;
constexpr float oneTurnTime = .5f;
rfct::animationBuffer animBuffer;

namespace rfct {
	void onCollision_Enemy_StaticObj(entity enemy, entity collidedWith, glm::vec2 resolution) {
		ecs::get().get<positionComponent>(enemy).position += resolution;
		velocityComponent& vel = ecs::get().get<velocityComponent>(enemy);
		if (resolution.x != 0) {
			vel.velocity.x = 0.f;
			enemyComponent& enComp = ecs::get().get<enemyComponent>(enemy);
			if (enComp.turningTime == 0.f)
				enComp.turningTime = oneTurnTime;
		}
		else if (resolution.y != 0) {
			vel.velocity.y = 0.f;

		}
	}

	void onCollision_Enemy_DynamicObj(entity enemy, entity collidedWith) {
		entt::registry& reg = ecs::get();
		if (reg.get<dynamicObjectTypeComponent>(collidedWith).passable)
			return;

		const auto& playerCol = reg.get<dynamicBoxColliderComponent>(collidedWith);
		const auto& playerPos = reg.get<positionComponent>(collidedWith);
		const auto& enemyCol = reg.get<dynamicBoxColliderComponent>(enemy);
		auto& enemyPos = reg.get<positionComponent>(enemy);

		glm::vec2 resolution = ResolveAABBCollision(
			enemyPos.position + enemyCol.min,
			enemyPos.position + enemyCol.max,
			playerCol.min + playerPos.position,
			playerCol.max + playerPos.position
		);

		enemyPos.position += resolution;

		auto& vel = reg.get<velocityComponent>(enemy);

		if (resolution.y == 0) {
			vel.velocity.x = 0.f;

			auto& enComp = reg.get<enemyComponent>(enemy);
			if (enComp.turningTime == 0.f)
				enComp.turningTime = oneTurnTime;
		}
		else if (resolution.x == 0) {
			vel.velocity.y = 0.f;
		}

		if (reg.get<dynamicObjectTypeComponent>(collidedWith).type == dynamicObjectType::Player) {
			if (!(resolution.x == 0 && resolution.y == 0)) {
				reg.get<playerLifeComponent>(collidedWith).alive = false;
			}
		}
	}
};

namespace rfct {
	void enemies::initSystem() {
		RFCT_PROFILE_FUNCTION();
		animBuffer.init(10000);
	}

	void enemies::spawnData(scene* s, sceneSerializedData* sd) {
		RFCT_PROFILE_FUNCTION();
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
			loadAnimation(singleAnimName + ".txt", &eComp.walkFrameAnim, &animBuffer, ssboMatrixIndex);

			iss >> singleAnimName;
			loadAnimation(singleAnimName + ".txt", &eComp.turnFrameAnim, &animBuffer, ssboMatrixIndex);

			iss >> singleAnimName;
			loadAnimation(singleAnimName + ".txt", &eComp.dieFrameAnim, &animBuffer, ssboMatrixIndex);

			eComp.animIndex = 0;
			eComp.frameIndex = 0;
			entt::registry& reg = ecs::get();
			entity enemy = ecs::get().create();
			reg.emplace<dynamicSSBOIndexComponent>(enemy, dynamicSSBOIndexComponent{ { ssboMatrixIndex } });
			reg.emplace<rotationComponent>(enemy, rotationComponent{ trans.rot });
			reg.emplace<scaleComponent>(enemy, scaleComponent{ trans.scale });
			reg.emplace<positionComponent>(enemy, positionComponent{ trans.pos });
			reg.emplace<gravityComponent>(enemy, gravityComponent{ 0.97, true, 3.f });
			reg.emplace<velocityComponent>(enemy, velocityComponent{ { glm::vec2(0.f, 0.f) } });
			reg.emplace<dynamicBoxColliderComponent>(enemy, dynamicBoxColliderComponent{ bounds });
			reg.emplace<staticObjCollisionCallbackComponent>(enemy, staticObjCollisionCallbackComponent{ colCallback });
			reg.emplace<dynamicObjCollisionCallbackComponent>(enemy, dynamicObjCollisionCallbackComponent{ dynColCallback });
			reg.emplace<enemyComponent>(enemy, enemyComponent{ eComp });
			reg.emplace<dynamicObjectTypeComponent>(enemy, dynamicObjectTypeComponent{ dynamicObjectType::Enemy, false });

		}
	};

	void enemies::updateVisuals(const frameContext* ctx) {
		RFCT_PROFILE_FUNCTION();
		auto enemyQuery = ecs::get().view<velocityComponent, positionComponent, scaleComponent, enemyComponent>();
		for (auto [ent, vel, pos, sc, en] : enemyQuery.each()) {
			ctx->scene->updateTransformData(ctx, ent);
		}
	};

	void enemies::updateSystem(frameContext* ctx) {
		RFCT_PROFILE_FUNCTION();
		auto enemyQuery = ecs::get().view<velocityComponent, positionComponent, scaleComponent, enemyComponent>();
		for (auto [ent, vel, pos, sc, en] : enemyQuery.each()) {
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
					// switch sides
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
		};
	};

	void enemies::cleanupSystem() {
		RFCT_PROFILE_FUNCTION();
		animBuffer.cleanupBuffer();
	}

	void enemies::drawFrameAnimSprites(vk::CommandBuffer& cmd, frameContext* ctx) {
		RFCT_PROFILE_FUNCTION();
		vk::Buffer vertexBuffers[] = { animBuffer.getBuffer() };

		auto enemyQuery = ecs::get().view<velocityComponent, positionComponent, scaleComponent, enemyComponent>();
		for (auto [ent, vel, pos, sc, en] : enemyQuery.each()) {

			frameAnimation* currentAnim = (frameAnimation*)((char*)&en.walkFrameAnim + (en.animIndex * sizeof(frameAnimation)));
			vk::DeviceSize offsets[] = { currentAnim->bufferOffsetInBytes + en.bufferOffset };
			cmd.bindVertexBuffers(0, 1, vertexBuffers, offsets);
			cmd.draw(currentAnim->trianglesPerFrame[en.animIndex] * 3, 1, 0, 0);

		};
	}
}
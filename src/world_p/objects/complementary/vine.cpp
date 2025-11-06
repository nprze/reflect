#include "vine.h"
#include "world_p/scene.h"
#include "world_p/ecs.h"
#include "world_p/transform.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include <random>
//debug
#include "renderer_p/debug/debug_draw.h"


#include "world_p/physics/physics.h"

#define RFCT_VINE_CONSTRAINS_ITERATIONS 10

float len(const glm::vec2& vector) {
	return std::sqrt((vector.x * vector.x) + (vector.y * vector.y));
}
glm::vec3 getColorWithFluctuate(float maxFluct = 0.2f, const glm::vec3& basicColor = glm::vec3(0.7098f, 0.9020f, 0.1137f)) {
	static std::mt19937 gen(std::random_device{}());
	std::uniform_real_distribution<float> dist(-maxFluct * 0.5f, maxFluct * 0.5f);
	float fluc = dist(gen);
	return basicColor + glm::vec3{ fluc, fluc, fluc };
}

namespace rfct {

	constexpr glm::vec2 vineGravity = { 0.f, -15.f };

	entity vineClosestToPlayer = entt::null;
	int nearestVineEdgeToPlayerIndex;
	glm::vec2 nearestVineEdgeToPlayerPosition;

	void onCollision_Vine_StaticObj(entity vineEntity, entity collidedWith, glm::vec2 resolution) {
		// narrow phase
		entt::registry& reg = ecs::get();
 		auto& vinePosCom = reg.get<vinePositionsComponent>(vineEntity);
		glm::vec2 vineBasePos = reg.get<positionComponent>(vineEntity).position;
		 
		const glm::vec2& objectMin = reg.get<staticBoxColliderComponent>(collidedWith).min;
		const glm::vec2 & objectMax = reg.get<staticBoxColliderComponent>(collidedWith).max;

		std::vector<glm::vec2>& positions = vinePosCom.positions;

		for (glm::vec2& p : positions) {
			glm::vec2 worldPos = vineBasePos + p;

			if (worldPos.x >= objectMin.x && worldPos.x <= objectMax.x &&
				worldPos.y >= objectMin.y && worldPos.y <= objectMax.y) {

				float left = worldPos.x - objectMin.x;
				float right = objectMax.x - worldPos.x;
				float bottom = worldPos.y - objectMin.y;
				float top = objectMax.y - worldPos.y;

				float minPen = std::min({ left, right, bottom, top });

				if (minPen == left) {
					worldPos.x = objectMin.x;
				}
				else if (minPen == right) {
					worldPos.x = objectMax.x;
				}
				else if (minPen == bottom) {
					worldPos.y = objectMin.y;
				}
				else { // top
					worldPos.y = objectMax.y;
				}

				p = worldPos - vineBasePos;
			}
		}
	}
	void onCollision_Vine_DynamicObj(entity vineEntity, entity collidedWith) {
		// narrow phase
		entt::registry& reg = ecs::get();
		if (reg.get<dynamicObjectTypeComponent>(collidedWith).passable)
			return;

		if (reg.get<vineStateComponent>(vineEntity).holdingToThis)
			return; // update separately

		glm::vec2 playerMin =
			reg.get<dynamicBoxColliderComponent>(collidedWith).min +
			reg.get<positionComponent>(collidedWith).position;

		glm::vec2 playerMax =
			reg.get<dynamicBoxColliderComponent>(collidedWith).max +
			reg.get<positionComponent>(collidedWith).position;

		auto& vinePosCom = reg.get<vinePositionsComponent>(vineEntity);
		auto& vineBasePos = reg.get<positionComponent>(vineEntity).position;


		std::vector<glm::vec2>& positions = vinePosCom.positions;

		for (glm::vec2& p : positions) {
			glm::vec2 worldPos = vineBasePos + p;

			if (worldPos.x >= playerMin.x && worldPos.x <= playerMax.x &&
				worldPos.y >= playerMin.y && worldPos.y <= playerMax.y) {

				float left = worldPos.x - playerMin.x;
				float right = playerMax.x - worldPos.x;
				float bottom = worldPos.y - playerMin.y;
				float top = playerMax.y - worldPos.y;

				float minPen = std::min({ left, right, bottom, top });

				if (minPen == left) {
					worldPos.x = playerMin.x;
				}
				else if (minPen == right) {
					worldPos.x = playerMax.x;
				}
				else if (minPen == bottom) {
					worldPos.y = playerMin.y;
				}
				else { // top
					worldPos.y = playerMax.y;
				}

				p = worldPos - vineBasePos;
			}
		}
	}
	std::pair<glm::vec2, int> getNearestEdgePos(const glm::vec2& PlayerPos, entity vine)
	{
		glm::vec2 posMin = glm::vec2(FLT_MAX);
		float distMin = FLT_MAX;
		int curVine = 0;
		int vineIndex = 0; 
		for (const glm::vec2& edgePos : ecs::get().get<vinePositionsComponent>(vine).positions) {
			const auto& vinePos = ecs::get().get<positionComponent>(vine).position;

			float dist = len(PlayerPos - (edgePos + vinePos));
			if (dist < distMin) {
				posMin = edgePos + vinePos;
				distMin = dist;
				vineIndex = curVine;
			}

			curVine++;
		}
		return { posMin, vineIndex };
	}

	void constructBoundingBox(dynamicBoxColliderComponent& boundingBox, const vinePositionsComponent& vinePos)
	{
		boundingBox.min = { FLT_MAX, FLT_MAX };
		boundingBox.max = { FLT_MIN, FLT_MIN };
		for (const glm::vec2& pos : vinePos.positions) {
			boundingBox.max.x = std::max(pos.x, boundingBox.max.x);
			boundingBox.max.y = std::max(pos.y, boundingBox.max.y);

			boundingBox.min.x = std::min(pos.x, boundingBox.min.x);
			boundingBox.min.y = std::min(pos.y, boundingBox.min.y);
		}
	}
	glm::vec2 simulateVinePlayerIsHolding(entity player,entity vineEntity, int vineEdgeIndex, const frameContext* fc)
	{
		entt::registry& reg = ecs::get();
		auto& vinePosCom = reg.get<vinePositionsComponent>(vineEntity);
		auto vineBasePos = reg.get<positionComponent>(vineEntity).position;

		std::vector<glm::vec2>& positions = reg.get<vinePositionsComponent>(vineEntity).positions;
		std::vector<glm::vec2>& previousPositions = reg.get<vinePositionsComponent>(vineEntity).previousPosition;

		glm::vec2 playerVel = reg.get<inputVelocityComponent>(player).velocity;

		glm::vec2 desiredMove = playerVel * 0.1f * fixedDeltaTime;
		desiredMove.y = -0.1f;

		glm::vec2 playerPos = reg.get<positionComponent>(player).position;

		glm::vec2 whereWeWantTheEdgeIndexToBeAtTheEnd = playerPos + desiredMove;
		glm::vec2 generalDirection = whereWeWantTheEdgeIndexToBeAtTheEnd - (positions[vineEdgeIndex] + vineBasePos);


		for (uint8_t i = 0; i < fc->fixedUpdateTimes; ++i) {

			positions[vineEdgeIndex] += generalDirection * 0.2f;

			for (uint32_t j = 0; j < positions.size(); ++j) {
				glm::vec2 vel = positions[j] - previousPositions[j];
				previousPositions[j] = positions[j];
				vel *= 0.99f;
				positions[j] += vel;
				positions[j] += glm::vec2{ 0.f,-15.f } *fixedDeltaTime * fixedDeltaTime;
			}

			positions[0] = { 0.f,-.01f };
			previousPositions[0] = { 0.f,-0.01f };

			for (int iter = 0; iter < RFCT_VINE_CONSTRAINS_ITERATIONS; ++iter) {
				for (uint32_t i = 1; i < positions.size(); ++i) {

					glm::vec2 dir = positions[i] - positions[i - 1];
					float dist = glm::length(dir);
					float diff = (dist - ecs::get().get<vineLenghtComponent>(vineEntity).oneBoneLenght) / dist;

					positions[i - 1] += dir * 0.5f * diff;
					positions[i] -= dir * 0.5f * diff;
				}
				positions[0] = glm::vec2(0.f, -.01f);
			}
		}
		constructBoundingBox(ecs::get().get<dynamicBoxColliderComponent>(vineEntity), ecs::get().get<vinePositionsComponent>(vineEntity));
		return positions[vineEdgeIndex] + vineBasePos;
	}
}

namespace rfct {

	void vines::initSystem() {

	}
	void vines::spawnData(scene* s, sceneSerializedData* sd) {
		for (vineInfo& vi : sd->vines) {

			vinePositionsComponent vpCom = {};
			vineBasePositionsComponent vbpCom = {};
			vpCom.positions.reserve(vi.numEdges);
			vpCom.previousPosition.reserve(vi.numEdges);
			vbpCom.basePositions.reserve(vi.numEdges);
			glm::vec2 start = { 0.f,0.f };
			glm::vec2 end = vi.end - vi.start;
			glm::vec2 dir = glm::normalize(end);
			float oneLineLen = len(end) / (vi.numEdges - 1);
			for (uint32_t i = 0; i < vi.numEdges; i++) {
				vpCom.positions.push_back((i * oneLineLen) * dir);
				vpCom.previousPosition.push_back((i * oneLineLen) * dir);
				vbpCom.basePositions.push_back((i * oneLineLen) * dir);
			}

			staticObjCollisionCallbackComponent colCallback;
			colCallback.handler = onCollision_Vine_StaticObj;
			dynamicObjCollisionCallbackComponent dynColCallback;
			dynColCallback.handler = onCollision_Vine_DynamicObj;

			vineVerticesComponent verts;
			verts.vertices.resize(12 * (vi.numEdges - 1));
			
			// set vertices colors
			for (uint32_t i = 0; i < (vi.numEdges - 1); ++i) {
				glm::vec3 black = { 0.f,0.f,0.f };
				verts.vertices[i * 12 + 0].color = black;
				verts.vertices[i * 12 + 1].color = black;
				verts.vertices[i * 12 + 2].color = black;
				verts.vertices[i * 12 + 3].color = black;
				verts.vertices[i * 12 + 4].color = black;
				verts.vertices[i * 12 + 5].color = black;

				glm::vec3 fluctuate = getColorWithFluctuate();
				verts.vertices[i * 12 + 6].color = fluctuate;
				verts.vertices[i * 12 + 7].color = fluctuate;
				verts.vertices[i * 12 + 8].color = fluctuate;

				fluctuate = getColorWithFluctuate();
				verts.vertices[i * 12 + 9].color = fluctuate;
				verts.vertices[i * 12 + 10].color = fluctuate;
				verts.vertices[i * 12 + 11].color = fluctuate;
			}
			glm::mat4 transform = glm::translate(glm::mat4(1.f), { vi.start, 0.f });
			objectLocation ol = s->getRenderData().addDynamicObject(&verts.vertices, &transform);

			frameContext simpleCtx = {};
			for (uint8_t i = 1; i < RFCT_FRAMES_IN_FLIGHT; i++) {
				simpleCtx.frame = i;
				s->getRenderData().updateMat(&simpleCtx, ol.indexInSSBO, &transform);
			}

			entt::registry& reg = ecs::get();

			entity e = ecs::get().create();

			reg.emplace<dynamicSSBOIndexComponent>(e, dynamicSSBOIndexComponent{ { ol.indexInSSBO } });
			reg.emplace<vertexRenderInfoComponent>(e, vertexRenderInfoComponent{ ol.verticesCount, ol.vertexBufferOffset });

			reg.emplace<positionComponent>(e, positionComponent{ { vi.start } });
			reg.emplace<gravityComponent>(e, gravityComponent{ 0.f, false, 0.f });
			reg.emplace<velocityComponent>(e, velocityComponent{ { 0.f, 0.f } });

			reg.emplace<staticObjCollisionCallbackComponent>(e, staticObjCollisionCallbackComponent{ colCallback });
			reg.emplace<dynamicObjCollisionCallbackComponent>(e, dynamicObjCollisionCallbackComponent{ dynColCallback });
			reg.emplace<dynamicBoxColliderComponent>(e, dynamicBoxColliderComponent{ {} });
			reg.emplace<dynamicObjectTypeComponent>(e, dynamicObjectTypeComponent{ { dynamicObjectType::Vine } });

			reg.emplace<vinePositionsComponent>(e, vinePositionsComponent{ vpCom });
			reg.emplace<vineBasePositionsComponent>(e, vineBasePositionsComponent{ vbpCom });
			reg.emplace<vineStateComponent>(e, vineStateComponent{ { false } });
			reg.emplace<vineLenghtComponent>(e, vineLenghtComponent{ { oneLineLen } });
			reg.emplace<vineVerticesComponent>(e, vineVerticesComponent{ verts });

			constructBoundingBox(reg.get<dynamicBoxColliderComponent>(e), reg.get<vinePositionsComponent>(e));
		}
		nearestVineEdgeToPlayerIndex = -1;
	};
	void vines::resetLevel(const frameContext* ctx) {
		auto vineQuery = ecs::get().view<vineStateComponent, vinePositionsComponent, vineLenghtComponent, dynamicBoxColliderComponent, positionComponent>();
		for (auto [ent, sc, positions, en, boc, pos] : vineQuery.each()) {

			std::vector<glm::vec2>& previousPositions = positions.previousPosition;
			std::vector<glm::vec2>& basePositions = ecs::get().get<vineBasePositionsComponent>(ent).basePositions;

			for (uint32_t i = 0; i < positions.positions.size(); ++i) {
				positions.positions[i] = basePositions[i];
				previousPositions[i] = basePositions[i];
			}
			};
	}
	/*void vines::onLevelSwitch(scene* scen)
	{
		auto vineQuery = ecs::get().view<vineStateComponent>();
		for (auto [ent, sc] : vineQuery.each()) {
			scen->deleteDynamicEntity(ent);
		}
	}
	;*/
	void vines::updateVisuals(const frameContext* ctx) {

		auto vineQuery = ecs::get().view<vinePositionsComponent, vineVerticesComponent, positionComponent, vertexRenderInfoComponent>();
		for (auto [ent, pos, verts, posComp, renInfo] : vineQuery.each()) {
			std::vector<glm::vec2>& positions = pos.positions;
			glm::vec2 start = posComp.position;
			glm::vec3 white = { 1.f,1.f,1.f };
			glm::vec3 blue = { 0.f,0.f,1.f };
			glm::vec3 yellow = { 1.f,1.f,0.f };

			uint32_t segmentCount = positions.size() - 1;


			float thickness = 0.1f;

			// official
			for (uint32_t i = 0; i < segmentCount; i++) {
				glm::vec2 p0 = positions[i];
				glm::vec2 p1 = positions[i + 1];

				glm::vec2 dir = glm::normalize(p1 - p0);

				glm::vec2 normal = glm::vec2(-dir.y, dir.x) * thickness * 0.5f;
				float lenNormal = glm::length(normal);

				constexpr float between = 0.75f;
				constexpr float oneMinusBetween = (float)(1.f - between);


				// background triangles
				glm::vec3 bg0 = glm::vec3(p0 - (normal * (1 + oneMinusBetween)) - dir * lenNormal * oneMinusBetween, 0.1f);
				glm::vec3 bg1 = glm::vec3(p0 + (normal * (1 + oneMinusBetween)) - dir * lenNormal * oneMinusBetween, 0.1f);
				glm::vec3 bg2 = glm::vec3(p1 - (normal * (1 + oneMinusBetween)) + dir * lenNormal * oneMinusBetween, 0.1f);
				glm::vec3 bg3 = glm::vec3(p1 + (normal * (1 + oneMinusBetween)) + dir * lenNormal * oneMinusBetween, 0.1f);

				verts.vertices[i * 12 + 0].pos = bg0;
				verts.vertices[i * 12 + 1].pos = bg1;
				verts.vertices[i * 12 + 2].pos = bg2;

				verts.vertices[i * 12 + 3].pos = bg1;
				verts.vertices[i * 12 + 4].pos = bg2;
				verts.vertices[i * 12 + 5].pos = bg3;



				// color triangles
				glm::vec3 v0 = glm::vec3(p0 - normal, 0.1f);
				glm::vec3 v1 = glm::vec3(p0 + normal * between, 0.1f);
				glm::vec3 v2 = glm::vec3(p0 + normal, 0.1f);
				glm::vec3 v3 = glm::vec3(p1 - normal, 0.1f);
				glm::vec3 v4 = glm::vec3(p1 - normal * between, 0.1f);
				glm::vec3 v5 = glm::vec3(p1 + normal, 0.1f);


				verts.vertices[i * 12 + 6].pos = v2;
				verts.vertices[i * 12 + 7].pos = v4;
				verts.vertices[i * 12 + 8].pos = v5;

				verts.vertices[i * 12 + 9].pos = v0;
				verts.vertices[i * 12 + 10].pos = v1;
				verts.vertices[i * 12 + 11].pos = v3;

				ctx->scene->getRenderData().updateDynamicVertices(ctx, renInfo.vertexBufferOffset, verts.vertices.data(), verts.vertices.size() * sizeof(Vertex));
			}
		};
	};
	void vines::updateSystem(frameContext* ctx) {
		if (ctx->fixedUpdateTimes) {
			if (nearestVineEdgeToPlayerIndex != -1) {
				if (ecs::get().get<vineStateComponent>(vineClosestToPlayer).holdingToThis) {
					ecs::get().get<positionComponent>(ctx->scene->getPlayer()).position = simulateVinePlayerIsHolding(ctx->scene->getPlayer(), vineClosestToPlayer, nearestVineEdgeToPlayerIndex, ctx);
				}
			}

			auto vineQuery = ecs::get().view<vineStateComponent, vinePositionsComponent, vineLenghtComponent, dynamicBoxColliderComponent, positionComponent>();
			for (auto [ent, sc, pos, vl, boc, position] : vineQuery.each()) {
				if (sc.holdingToThis)return;
				std::vector<glm::vec2>& positions = pos.positions;
				std::vector<glm::vec2>& previousPositions = pos.previousPosition;
				float oneBoneLenght = vl.oneBoneLenght;
				for (uint8_t i = 0; i < ctx->fixedUpdateTimes; ++i) {
					for (uint32_t j = 0; j < positions.size(); ++j) {
						glm::vec2 vel = positions[j] - previousPositions[j];
						previousPositions[j] = positions[j];
						vel *= 0.99f;
						positions[j] += vel;
						positions[j] += vineGravity * fixedDeltaTime * fixedDeltaTime;
					}

					positions[0] = { 0.f,-.01f };
					previousPositions[0] = { 0.f,-0.01f };

					for (int iter = 0; iter < RFCT_VINE_CONSTRAINS_ITERATIONS; ++iter) {
						for (uint32_t i = 1; i < positions.size(); ++i) {

							glm::vec2 dir = positions[i] - positions[i - 1];
							float dist = glm::length(dir);
							float diff = (dist - oneBoneLenght) / dist;

							positions[i - 1] += dir * 0.5f * diff;
							positions[i] -= dir * 0.5f * diff;
						}
						positions[0] = glm::vec2(0.f, -.01f);
					}

				}
				constructBoundingBox(boc, pos);
				};
		}
	};
	void vines::onStartHolding(nearestObject& nearest)
	{
		vineClosestToPlayer = nearest.object;
		ecs::get().get<vineStateComponent>(vineClosestToPlayer).holdingToThis = true;
		nearestVineEdgeToPlayerIndex = nearest.vineIndex;
	}
	void vines::onEndHolding()
	{
		nearestVineEdgeToPlayerIndex = -1;
		if (vineClosestToPlayer != entt::null) {
			ecs::get().get<vineStateComponent>(vineClosestToPlayer).holdingToThis = false;
			vineClosestToPlayer = entt::null;
		}
	}
}


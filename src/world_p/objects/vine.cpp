	#include "vine.h"
	#include "world_p/scene.h"
	#include "world_p/ecs.h"
	#include "world_p/transform.h"
	#include "renderer_p/debug/debug_draw.h"

	namespace rfct {
		void onCollision_Vine_StaticObj(entity vineEntity, entity collidedWith, glm::vec2 resolution) {
			// narrow phase

			auto vinePosCom = vineEntity.get_mut<vinePositionsComponent>();
			auto vineBasePos = vineEntity.get<positionComponent>()->position;

			const glm::vec2& objectMin = collidedWith.get<staticBoxColliderComponent>()->min;
			const glm::vec2& objectMax = collidedWith.get<staticBoxColliderComponent>()->max;

			std::vector<glm::vec2>& positions = vinePosCom->positions;

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
			if(collidedWith.get<dynamicObjectTypeComponent>()->type != dynamicObjectType::Player)return;
			glm::vec2 playerMin = collidedWith.get_mut<dynamicBoxColliderComponent>()->min + collidedWith.get<positionComponent>()->position;
			glm::vec2 playerMax = collidedWith.get_mut<dynamicBoxColliderComponent>()->max + collidedWith.get<positionComponent>()->position;

			auto vinePosCom = vineEntity.get_mut<vinePositionsComponent>();
			auto vineBasePos = vineEntity.get<positionComponent>()->position;

			std::vector<glm::vec2>& positions = vinePosCom->positions;

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
	}
	float len(const glm::vec2& vector) {
		return std::sqrt((vector.x * vector.x) + (vector.y * vector.y));
	}
	rfct::vine::vine(const glm::vec2& startArg, const glm::vec2& endArg, const int numEdges, scene* parentScene)
	{
		vinePositionsComponent vpCom = {};
		vpCom.positions.reserve(numEdges);
		vpCom.previousPosition.reserve(numEdges);
		glm::vec2 start = { 0.f,0.f };
		glm::vec2 end = endArg - startArg;
		glm::vec2 dir = glm::normalize(end);
		float oneLineLen = len(end) / (numEdges - 1);
		oneBoneLenght = oneLineLen;
		for (uint32_t i = 0; i < numEdges; i++) {
			vpCom.positions.push_back((i * oneLineLen) * dir);
			vpCom.previousPosition.push_back((i * oneLineLen) * dir);
		}

		staticObjCollisionCallbackComponent colCallback;
		colCallback.handler = onCollision_Vine_StaticObj;
		dynamicObjCollisionCallbackComponent dynColCallback;
		dynColCallback.handler = onCollision_Vine_DynamicObj;




		m_vineEntity = ecs::get().entity<>()
			.child_of(parentScene->sceneEntity)
			//.set<dynamicSSBOIndexComponent>({ 0 })
			//.set<rotationComponent>({})
			.set<vinePositionsComponent>(vpCom)
			.set<positionComponent>({ startArg })
			.set<gravityComponent>({0.f,false,0.f})
			.set<velocityComponent>({ glm::vec3(0.f,0.f,0.f) })
			.set<staticObjCollisionCallbackComponent>(colCallback)
			.set<dynamicObjCollisionCallbackComponent>(dynColCallback)
			.set<dynamicBoxColliderComponent>({})
			.set<dynamicObjectTypeComponent>({ dynamicObjectType::Vine });
		constructBoundingBox();
	}

	void rfct::vine::update(const frameContext* fc)
	{
		glm::vec2 start = m_vineEntity.get<positionComponent>()->position;
		glm::vec3 white = { 1.f,1.f,1.f };
		glm::vec3 blue = { 0.f,0.f,1.f };
		debugLine* lines = debugDraw::requestLines(m_vineEntity.get<vinePositionsComponent>()->positions.size() - 1);
		for (uint32_t i = 0; i < m_vineEntity.get<vinePositionsComponent>()->positions.size() - 1;i++) {
			lines[i].vertices[0].pos = glm::vec3({ start + m_vineEntity.get<vinePositionsComponent>()->positions[i], 0.f });
			lines[i].vertices[1].pos = glm::vec3({ start + m_vineEntity.get<vinePositionsComponent>()->positions[i + 1], 0.f });
			if (i % 2) {
				lines[i].vertices[0].color = white;
				lines[i].vertices[1].color = white;
			}
			else {
				lines[i].vertices[0].color = blue;
				lines[i].vertices[1].color = blue;
			}
		}
		if (fc->fixedUpdateTimes) {
			std::vector<glm::vec2>& positions = m_vineEntity.get_mut<vinePositionsComponent>()->positions;
			std::vector<glm::vec2>& previousPositions = m_vineEntity.get_mut<vinePositionsComponent>()->previousPosition;
			for (uint8_t i = 0; i < fc->fixedUpdateTimes; ++i) {
				for (uint32_t j = 0; j < positions.size();++j) {
					glm::vec2 vel = positions[j] - previousPositions[j];
					previousPositions[j] = positions[j];
					vel *= 0.97f;
					positions[j] += vel;
					positions[j] += m_gravity * fixedDeltaTime * fixedDeltaTime;
				}

				positions[0] = { 0.f,-.0001f };
				previousPositions[0] = { 0.f,-.0001f };

				for (int iter = 0; iter < 200; ++iter) {
					for (uint32_t i = 1; i < positions.size(); ++i) {

						glm::vec2 dir = positions[i] - positions[i - 1];
						float dist = glm::length(dir);
						float diff = (dist - oneBoneLenght) / dist;

						positions[i - 1] += dir * 0.5f * diff;
						positions[i] -= dir * 0.5f * diff;
					}
					positions[0] = glm::vec2(0.f,-0.0001f);
				}

			}
			constructBoundingBox();
		}
	}

	void rfct::vine::constructBoundingBox()
	{
		dynamicBoxColliderComponent* boundingBox = m_vineEntity.get_mut<dynamicBoxColliderComponent>();
		boundingBox->min = { FLT_MAX, FLT_MAX };
		boundingBox->max = { FLT_MIN, FLT_MIN };
		for (const glm::vec2& pos : m_vineEntity.get<vinePositionsComponent>()->positions) {
			boundingBox->max.x = std::max(pos.x, boundingBox->max.x);
			boundingBox->max.y = std::max(pos.y, boundingBox->max.y);

			boundingBox->min.x = std::min(pos.x, boundingBox->min.x);
			boundingBox->min.y = std::min(pos.y, boundingBox->min.y);
		}
	}

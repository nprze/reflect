#include "vine.h"
#include "world_p/scene.h"
#include "world_p/ecs.h"
#include "world_p/transform.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/components.h"
#include <random>

#define RFCT_VINE_CONSTRAINS_ITERATIONS 10

float len(const glm::vec2& vector) {
	return std::sqrt((vector.x * vector.x) + (vector.y * vector.y));
}
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
		if(collidedWith.get<dynamicObjectTypeComponent>()->type != dynamicObjectType::Player) return;
		if (vineEntity.get<vineStateComponent>()->holdingToThis) return; // update separately
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
	std::pair<glm::vec2, int> getNearestEdgePos(const glm::vec2& PlayerPos, entity vine)
	{
		glm::vec2 posMin = glm::vec2(FLT_MAX);
		float distMin = FLT_MAX;
		int curVine = 0;
		int vineIndex = 0;
		for (const glm::vec2& edgePos : vine.get<vinePositionsComponent>()->positions) {
			if (len(PlayerPos - (edgePos + vine.get<positionComponent>()->position)) < distMin) {
				posMin = edgePos + vine.get<positionComponent>()->position;
				distMin = len(PlayerPos - (edgePos + vine.get<positionComponent>()->position));
				vineIndex = curVine;
			}
			curVine++;
		}
		return { posMin, vineIndex };
	}

	glm::vec2 simulateVinePlayerIsHolding(entity player,entity vineEntity, int vineEdgeIndex, const frameContext* fc)
	{
		auto vinePosCom = vineEntity.get_mut<vinePositionsComponent>();
		auto vineBasePos = vineEntity.get<positionComponent>()->position;

		std::vector<glm::vec2>& positions = vineEntity.get_mut<vinePositionsComponent>()->positions;
		std::vector<glm::vec2>& previousPositions = vineEntity.get_mut<vinePositionsComponent>()->previousPosition;

		glm::vec2 playerVel = glm::vec2(player.get<inputVelocityComponent>()->velocity);
		glm::vec2 desiredMove = playerVel * 0.1f * fixedDeltaTime;
		desiredMove.y = -.1f;

		glm::vec2 playerPos = player.get<positionComponent>()->position;
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
					float diff = (dist - vineEntity.get<vineLenghtComponent>()->oneBoneLenght) / dist;

					positions[i - 1] += dir * 0.5f * diff;
					positions[i] -= dir * 0.5f * diff;
				}
				positions[0] = glm::vec2(0.f, -.01f);
			}
		}
		constructBoundingBox(vineEntity);
		return positions[vineEdgeIndex] + vineBasePos;
	}
	void constructBoundingBox(entity vineEntity)
	{
		dynamicBoxColliderComponent* boundingBox = vineEntity.get_mut<dynamicBoxColliderComponent>();
		boundingBox->min = { FLT_MAX, FLT_MAX };
		boundingBox->max = { FLT_MIN, FLT_MIN };
		for (const glm::vec2& pos : vineEntity.get<vinePositionsComponent>()->positions) {
			boundingBox->max.x = std::max(pos.x, boundingBox->max.x);
			boundingBox->max.y = std::max(pos.y, boundingBox->max.y);

			boundingBox->min.x = std::min(pos.x, boundingBox->min.x);
			boundingBox->min.y = std::min(pos.y, boundingBox->min.y);
		}
	}
}
glm::vec3 getColorWithFluctuate(float maxFluct = 0.2f, const glm::vec3& basicColor = glm::vec3(0.7098f, 0.9020f, 0.1137f)) {
	static std::mt19937 gen(std::random_device{}());
	std::uniform_real_distribution<float> dist(-maxFluct * 0.5f, maxFluct * 0.5f);
	float fluc = dist(gen);
	return basicColor + glm::vec3{ fluc, fluc, fluc };
}

rfct::vine::vine(const glm::vec2& startArg, const glm::vec2& endArg, const int numEdges, scene* parentScene):m_vertices((numEdges-1) * 3 * 4, Vertex())
{
	vinePositionsComponent vpCom = {};
	vpCom.positions.reserve(numEdges);
	vpCom.previousPosition.reserve(numEdges);
	glm::vec2 start = { 0.f,0.f };
	glm::vec2 end = endArg - startArg;
	glm::vec2 dir = glm::normalize(end);
	float oneLineLen = len(end) / (numEdges - 1);
	for (uint32_t i = 0; i < numEdges; i++) {
		vpCom.positions.push_back((i * oneLineLen) * dir);
		vpCom.previousPosition.push_back((i * oneLineLen) * dir);
	}

	staticObjCollisionCallbackComponent colCallback;
	colCallback.handler = onCollision_Vine_StaticObj;
	dynamicObjCollisionCallbackComponent dynColCallback;
	dynColCallback.handler = onCollision_Vine_DynamicObj;


	// set vertices colors
	for (uint32_t i = 0; i < (numEdges - 1); ++i) {
		glm::vec3 black = { 0.f,0.f,0.f };
		m_vertices[i * 12 + 0].color = black;
		m_vertices[i * 12 + 1].color = black;
		m_vertices[i * 12 + 2].color = black;
		m_vertices[i * 12 + 3].color = black;
		m_vertices[i * 12 + 4].color = black;
		m_vertices[i * 12 + 5].color = black;

		glm::vec3 fluctuate = getColorWithFluctuate();
		m_vertices[i * 12 + 6].color = fluctuate;
		m_vertices[i * 12 + 7].color = fluctuate;
		m_vertices[i * 12 + 8].color = fluctuate;

		fluctuate = getColorWithFluctuate();
		m_vertices[i * 12 + 9].color = fluctuate;
		m_vertices[i * 12 + 10].color = fluctuate;
		m_vertices[i * 12 + 11].color = fluctuate;
	}
	glm::mat4 transform = glm::translate(glm::mat4(1.f), { startArg, 0.f});
	objectLocation ol = parentScene->m_RenderData.addDynamicObject(&m_vertices, &transform);

	frameContext simpleCtx = {};
	for (uint8_t i = 1; i < RFCT_FRAMES_IN_FLIGHT; i++) {
		simpleCtx.frame = i;
		parentScene->m_RenderData.updateMat(&simpleCtx, ol.indexInSSBO, &transform);
	}


	m_vineEntity = ecs::get().entity<>()
		.child_of(parentScene->sceneEntity)
		.set<dynamicSSBOIndexComponent>({ ol.indexInSSBO })
		.set<vertexRenderInfoComponent>({ ol.verticesCount, ol.vertexBufferOffset })
		.set<vinePositionsComponent>(vpCom)
		.set<positionComponent>({ startArg })
		.set<gravityComponent>({ 0.f,false,0.f })
		.set<velocityComponent>({ {0.f, 0.f} })
		.set<staticObjCollisionCallbackComponent>(colCallback)
		.set<dynamicObjCollisionCallbackComponent>(dynColCallback)
		.set<dynamicBoxColliderComponent>({})
		.set<dynamicObjectTypeComponent>({ dynamicObjectType::Vine })
		.set<vineStateComponent>({ false })
		.set<vineLenghtComponent>({ oneLineLen });
	constructBoundingBox(m_vineEntity);
}

void rfct::vine::update(const frameContext* fc)
{
	if (fc->fixedUpdateTimes) {
		if (m_vineEntity.get<vineStateComponent>()->holdingToThis)return;
		std::vector<glm::vec2>& positions = m_vineEntity.get_mut<vinePositionsComponent>()->positions;
		std::vector<glm::vec2>& previousPositions = m_vineEntity.get_mut<vinePositionsComponent>()->previousPosition;
		float oneBoneLenght = m_vineEntity.get<vineLenghtComponent>()->oneBoneLenght;
		for (uint8_t i = 0; i < fc->fixedUpdateTimes; ++i) {
			for (uint32_t j = 0; j < positions.size();++j) {
				glm::vec2 vel = positions[j] - previousPositions[j];
				previousPositions[j] = positions[j];
				vel *= 0.99f;
				positions[j] += vel;
				positions[j] += m_gravity * fixedDeltaTime * fixedDeltaTime;
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
				positions[0] = glm::vec2(0.f,-.01f);
			}

		}
		constructBoundingBox(m_vineEntity);
	}
}

void rfct::vine::draw(const frameContext* fc)
{

	glm::vec2 start = m_vineEntity.get<positionComponent>()->position;
	glm::vec3 white = { 1.f,1.f,1.f };
	glm::vec3 blue = { 0.f,0.f,1.f };
	glm::vec3 yellow = { 1.f,1.f,0.f };

	auto& positions = m_vineEntity.get<vinePositionsComponent>()->positions;
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


		m_vertices[i * 12 + 0].pos = bg0;
		m_vertices[i * 12 + 1].pos = bg1;
		m_vertices[i * 12 + 2].pos = bg2;

		m_vertices[i * 12 + 3].pos = bg1;
		m_vertices[i * 12 + 4].pos = bg2;
		m_vertices[i * 12 + 5].pos = bg3;



		// color triangles
		glm::vec3 v0 = glm::vec3(p0 - normal, 0.1f);
		glm::vec3 v1 = glm::vec3(p0 + normal * between, 0.1f);
		glm::vec3 v2 = glm::vec3(p0 + normal, 0.1f);
		glm::vec3 v3 = glm::vec3(p1 - normal, 0.1f);
		glm::vec3 v4 = glm::vec3(p1 - normal * between, 0.1f);
		glm::vec3 v5 = glm::vec3(p1 + normal, 0.1f);


		m_vertices[i * 12 + 6].pos = v2;
		m_vertices[i * 12 + 7].pos = v4;
		m_vertices[i * 12 + 8].pos = v5;

		m_vertices[i * 12 + 9].pos= v0;
		m_vertices[i * 12 + 10].pos = v1;
		m_vertices[i * 12 + 11].pos = v3;

		fc->scene->getRenderData().updateDynamicVertices(fc, m_vineEntity.get<vertexRenderInfoComponent>()->vertexBufferOffset, m_vertices.data(), m_vertices.size() * sizeof(Vertex));

	}
}
#pragma once
#include <glm/glm.hpp>
#include "context.h"
#include "world_p/components.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace rfct {
	std::pair<glm::vec2, int> getNearestEdgePos(const glm::vec2& PlayerPos, entity vine);
	glm::vec2 simulateVinePlayerIsHolding(entity player, entity vineEntity, int vineEdgeIndex, const frameContext* fc);
	void constructBoundingBox(entity vineEntity);
	class vine {
	public:
		vine(const glm::vec2& start, const glm::vec2& end, const int numEdges, scene* parentScene);
		void update(const frameContext* fc);
		void reset();
		void draw(const frameContext* fc);
	private:
		entity m_vineEntity;
		glm::vec2 m_gravity = { 0.f, -15.f };
		std::vector<Vertex> m_vertices;
	};
}
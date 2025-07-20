#pragma once
#include <glm/glm.hpp>
#include "context.h"
#include "world_p/components.h"

namespace rfct {
	class vine {
	public:
		vine(const glm::vec2& start, const glm::vec2& end, const int numEdges, scene* parentScene);
		void update(const frameContext* fc);
		void constructBoundingBox();
	private:
		entity m_vineEntity;
		glm::vec2 m_gravity = { 0.f, -5.f };
		float oneBoneLenght;
	};
}
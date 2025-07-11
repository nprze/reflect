#pragma once
#include <glm/glm.hpp>
namespace rfct {
	struct edge {
		glm::vec2 pos;
		glm::vec2 previousPos;
	};
	struct hairAnimation {
		void init(glm::vec2 offsetFromPlayerOrigin, float len, uint32_t numEdges);
		void update(const glm::vec2& playerVel, uint8_t dt);
		void draw(const glm::vec2& playerPos);

		uint32_t m_numEdges;
		float m_lenght;
		float m_oneBoneLenght;
		glm::vec2 m_offsetFromPlayerOrigin;
		glm::vec2 m_gravity = { 0.f, -5.f };

		std::vector<edge> m_edges;
	};
}

#include "hair_anim.h"
#include "renderer_p/debug/debug_draw.h"
void rfct::hairAnimation::init(glm::vec2 offsetFromPlayerOrigin, float len, uint32_t numEdges)
{
    m_offsetFromPlayerOrigin = offsetFromPlayerOrigin;
    m_lenght = len;
    m_oneBoneLenght = len / (numEdges - 1);
    m_numEdges = numEdges;

    m_edges.reserve(numEdges);
    m_edges.push_back({ m_offsetFromPlayerOrigin, m_offsetFromPlayerOrigin });
    for (uint32_t i = 1; i < m_numEdges; ++i) {
        m_edges.push_back({ m_edges[i - 1].pos - glm::vec2(0, m_oneBoneLenght), m_edges[i - 1].pos - glm::vec2(0, m_oneBoneLenght) });
    }
}

void rfct::hairAnimation::update(const glm::vec2& playerVel, uint8_t fixedUpdateTimes)
{

    for (uint8_t i = 0; i < fixedUpdateTimes; ++i) {
        for (auto& e : m_edges) {
            glm::vec2 vel = e.pos - e.previousPos;
            e.previousPos = e.pos;
            vel *= 0.97f;
            e.pos += vel;
            e.pos += m_gravity * fixedDeltaTime * fixedDeltaTime;
        }

        m_edges[0].pos = m_offsetFromPlayerOrigin;
        m_edges[0].previousPos = m_offsetFromPlayerOrigin;

        for (int iter = 0; iter < 20; ++iter) {
            for (uint32_t i = 1; i < m_numEdges; ++i) {

                glm::vec2 dir = m_edges[i].pos - m_edges[i - 1].pos;
                float dist = glm::length(dir);
                float diff = (dist - m_oneBoneLenght) / dist;

                m_edges[i - 1].pos += dir * 0.5f * diff;
                m_edges[i].pos -= dir * 0.5f * diff;
            }
            m_edges[0].pos = m_offsetFromPlayerOrigin;
        }

        for (auto& e : m_edges) {
            glm::vec2 vel = playerVel;

            if (playerVel.y < 0.0f) {
                vel.y *= 2.0f;
            }

            e.pos += vel * fixedDeltaTime * -0.1f;
        }
    }
}


void rfct::hairAnimation::draw(const glm::vec2& playerPos)
{
    debugLine* lines = debugDraw::requestLines(m_numEdges - 1);
    for (uint32_t i = 1; i < m_edges.size(); ++i) {
        lines[i - 1].vertices[0].pos = { m_edges[i - 1].pos + playerPos, 0.f };
        lines[i - 1].vertices[1].pos = { m_edges[i].pos + playerPos, 0.f };

        lines[i - 1].vertices[0].color = glm::vec3{ 1.f,1.f,1.f };
        lines[i - 1].vertices[1].color = glm::vec3{ 1.f,1.f,1.f };
    }
}
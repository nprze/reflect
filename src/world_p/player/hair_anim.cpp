#include "hair_anim.h"
#include <glm/gtc/matrix_transform.hpp>
#include "renderer_p/debug/debug_draw.h"

void rfct::hairAnimation::init(glm::vec2 offsetFromPlayerOrigin, float len, uint32_t numEdges)
{
    RFCT_ASSERT(numEdges == 4);
    m_offsetFromPlayerOrigin = offsetFromPlayerOrigin;
    m_lenght = len;
    m_oneBoneLenght = len / (numEdges - 1);
    m_numEdges = numEdges;


    m_edges.reserve(numEdges);
    m_edges.push_back({ m_offsetFromPlayerOrigin, m_offsetFromPlayerOrigin });
    for (uint32_t i = 1; i < m_numEdges; ++i) {
        m_edges.push_back({ m_edges[i - 1].pos - glm::vec2(0, m_oneBoneLenght), m_edges[i - 1].pos - glm::vec2(0, m_oneBoneLenght) });
    }


    // matrix setup
    m_right = glm::translate(glm::mat4(1), glm::vec3(-m_edges[0].pos, 0.0f));

    m_right = glm::rotate(m_right, 3.14f * 0.22f, glm::vec3(0.0f, 0.0f, 1.0f)); // 20 deg counterclockwise
    m_left = glm::rotate(m_right, -3.14f * 0.17f, glm::vec3(0.0f, 0.0f, 1.0f)); // 30 deg clockwise

    m_right = glm::translate(m_right, glm::vec3(m_edges[0].pos, 0.0f));
    m_left = glm::translate(m_left, glm::vec3(m_edges[0].pos, 0.0f));
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

void drawToPoints(rfct::debugTriangle* triangle, const std::vector<rfct::edge>& edges, const glm::vec2& playerPos, uint32_t index0, uint32_t index1, glm::vec3 multipliedPos) {

    triangle->vertices[0].pos = { edges[index0].pos + playerPos, 0.f };
    triangle->vertices[1].pos = { edges[index1].pos + playerPos, 0.f };
    triangle->vertices[2].pos = multipliedPos + glm::vec3(playerPos, 0.f);
}

void rfct::hairAnimation::draw(const glm::vec2& playerPos)
{
    /*
    debugLine* lines = debugDraw::requestLines(m_numEdges - 1);
    for (uint32_t i = 1; i < m_edges.size(); ++i) {
        lines[i - 1].vertices[0].pos = { m_edges[i - 1].pos + playerPos, 0.f };
        lines[i - 1].vertices[1].pos = { m_edges[i].pos + playerPos, 0.f };

        lines[i - 1].vertices[0].color = glm::vec3{ 1.f,1.f,1.f };
        lines[i - 1].vertices[1].color = glm::vec3{ 1.f,1.f,1.f };
    }*/


    debugTriangle* triangles = debugDraw::requestTriangles(6);
    
    glm::vec3 leftMult = glm::vec3(glm::vec4(m_edges[2].pos, 0.f, 1.f) * m_left);
    glm::vec3 rightMult = glm::vec3(glm::vec4(m_edges[2].pos, 0.f, 1.f) * m_right);

    for (uint32_t i = 0; i < 3; i++) {
        drawToPoints(&triangles[2*i], m_edges, playerPos, i, i+1, leftMult);
        drawToPoints(&triangles[2*i+1], m_edges, playerPos, i, i+1, rightMult);
    }

    glm::vec3 color = glm::vec3(0.8f, 1.f, 0.4f);
    for (uint32_t i = 0; i < 6; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            triangles[i].vertices[j].color = color;
        }
    }
}
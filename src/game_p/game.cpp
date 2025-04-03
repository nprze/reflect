#include "game.h"
#include "renderer_p\debug\debug_draw.h"
using namespace rfct;
game::game::game()
{
}

std::chrono::high_resolution_clock::time_point lastTime;
int frameCount = 0;
float fps = 0.0f;

void game::game::onUpdate(float dt)
{
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		std::vector<Vertex> vertices = {
			{{0.0f, -0.5f, 0.f}, {1.0f, 1.0f, 1.0f},0,0},
			{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f},0,0},
			{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f},0,0}
		};
		transformComponent tc = {};
		tc.position.x += 1.f;
		epicRotatingTriangle = world::getWorld().getCurrentScene().createDynamicRenderingEntity(&vertices, &tc);

	}
	transformComponent* tc = world::getWorld().getCurrentScene().getComponent<transformComponent>(epicRotatingTriangle);
	if (input::getInput().xAxis) {
		tc->position.x += 3 * input::getInput().xAxis * dt;
	}
	if (input::getInput().yAxis) {
		tc->position.y += 3 * input::getInput().yAxis * dt;
	}
	world::getWorld().getCurrentScene().updateTransformData(epicRotatingTriangle);
	/*
	debugTriangle* trig = debugDraw::requestTriangles(2);
	trig[0].vertices[0].pos = { -0.25f, -0.25f, 0.f };
	trig[0].vertices[1].pos = { 0.25f, 0.25f, 0.f };
	trig[0].vertices[2].pos = { -0.25f, 0.25f, 0.f };

	trig[1].vertices[0].pos = { -0.25f, -0.25f, 0.f };
	trig[1].vertices[1].pos = { 0.25f, -0.25f, 0.f };
	trig[1].vertices[2].pos = { 0.25f, 0.25f, 0.f };

	trig[0].vertices[0].color = { 1.0f, 1.0f, 1.0f };
	trig[0].vertices[1].color = { 1.0f, 1.0f, 1.0f };
	trig[0].vertices[2].color = { 1.0f, 1.0f, 1.0f };

	trig[1].vertices[0].color = { 1.0f, 1.0f, 1.0f };
	trig[1].vertices[1].color = { 1.0f, 1.0f, 1.0f };
	trig[1].vertices[2].color = { 1.0f, 1.0f, 1.0f };*/


	debugLine* line = debugDraw::requestLines(1);
	line[0].vertices[0].pos = { -0.25f, -0.25f, 0.f };
	line[0].vertices[1].pos = { 0.25f, -0.25f, 0.f };

	line[0].vertices[0].color = { 1.0f, 1.0f, 1.0f };
	line[0].vertices[1].color = { 1.0f, 1.0f, 1.0f };

	debugDraw::drawText("FPS: " + std::to_string(int(1/dt)), glm::vec2(0, 0), 0.2);
}

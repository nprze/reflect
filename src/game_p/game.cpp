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
	trig[1].vertices[2].color = { 1.0f, 1.0f, 1.0f };


	debugLine* line = debugDraw::requestLines(1);
	line[0].vertices[0].pos = { -0.25f, -0.25f, 0.f };
	line[0].vertices[1].pos = { 0.25f, -0.25f, 0.f };

	line[0].vertices[0].color = { 1.0f, 1.0f, 0.0f };
	line[0].vertices[1].color = { 1.0f, 0.0f, 1.0f };

	debugDraw::drawText("FPS: " + std::to_string(int(1/dt)), glm::vec2(0, 0), 0.2);
}

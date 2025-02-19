#include "game.h"

game::game::game()
{
}

void game::game::onUpdate()
{
	static uint32_t frame_count = 0;
	frame_count++;
	if (frame_count % 100 == 0) RFCT_TRACE("game update works as intended: update frame {0}", frame_count);
}

#pragma once
#include "world_p\components.h"
namespace game {
	class game {
	public:
		game();
		void onUpdate(float dt);
		rfct::Entity epicRotatingTriangle;
	};
}
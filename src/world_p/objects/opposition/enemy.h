#pragma once
#include "context.h"
#include "world_p/objects/object_system.h"

namespace vk { class CommandBuffer; }
namespace rfct {

	struct enemies : objectSystem {
		void initSystem();
		void spawnData(scene* s, sceneSerializedData* sd);
		void resetLevel(const frameContext* ctx);
		void updateVisuals(const frameContext* ctx);
		void updateSystem(frameContext* ctx);
		void cleanupSystem();

		void drawFrameAnimSprites(vk::CommandBuffer& cmd, frameContext* ctx);
		void rebuildQuery(entity scene);
	};
}
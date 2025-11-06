#pragma once
#include "context.h"
#include "world_p/objects/object_system.h"

namespace vk { class CommandBuffer; }
namespace rfct {

	struct enemies : objectSystem {
		void initSystem() override;
		void spawnData(scene* s, sceneSerializedData* sd) override;
		void resetLevel(const frameContext* ctx) override;
		//void onLevelSwitch(scene* scen) override;
		void updateVisuals(const frameContext* ctx) override;
		void updateSystem(frameContext* ctx) override;
		void cleanupSystem() override;

		void drawFrameAnimSprites(vk::CommandBuffer& cmd, frameContext* ctx);
	};
}
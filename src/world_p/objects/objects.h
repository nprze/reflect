#pragma once

#include "core/cigarettes.h"
#include "opposition/enemy.h"
#include "opposition/spikes.h"
#include "complementary/vine.h"
#include "complementary/npc.h"
#include "boosters/jump_booster.h"

namespace rfct{
	struct sceneSerializedData;
	class objectSystems {
	public:
		static objectSystems& get();
	public:
		void init();
		void cleanupBuffer();
		void loadSceneData(sceneSerializedData* serializeData, scene* parentScene);
		void systemsFixedUpdate(RfctFrameContext* fc);
		void updateVisuals(RfctFrameContext* fc);
		void customDrawObjects(vk::CommandBuffer& cmd, RfctFrameContext* ctx);
		void respawn(RfctFrameContext* fc);
		void onPlayerDash(RfctFrameContext* fc, const entity entityPlayer, const bool facingRight); // to be called before physics update
		void onStartHolding(playerState currentGameState, nearestObject& nearest);
		void onEndHolding();
	public:
		cigarettes m_cigSystem;
		enemies m_enemySystem;
		vines m_vineSystem;
		npcs m_npcSystem;
		spikes m_spikeSystem;
		jumpBoosters m_jumpBoostSystem;
	};
}
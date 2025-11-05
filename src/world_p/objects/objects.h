#pragma once

// surely good
#include "core/cigarettes.h"
#include "opposition/enemy.h"
#include "opposition/spikes.h"
#include "complementary/vine.h"
#include "complementary/npc.h"
#include "boosters/jump_booster.h"

namespace rfct{
	struct sceneSerializedData;
	struct objectSystems {
		void init();
		void cleanup();
		void loadSceneData(sceneSerializedData* serializeData, scene* parentScene);
		void update(frameContext* fc);
		void updateVisuals(frameContext* fc);
		void customDrawObjects(vk::CommandBuffer& cmd, frameContext* ctx);
		void respawn(frameContext* fc);
		
		void onPlayerDash(frameContext* fc, const entity entityPlayer, const bool facingRight); // to be called before physics update
		void onStartHolding(playerState state, nearestObject& nearest);
		void onEndHolding();



		
		// systems
		cigarettes m_cigSystem;
		enemies m_enemySystem;
		vines m_vineSystem;
		npcs m_npcSystem;
		spikes m_spikeSystem;
		jumpBoosters m_jumpBoostSystem;
	private:
		static objectSystems instance;
	public:
		static objectSystems& get() { return instance; }
	};
}
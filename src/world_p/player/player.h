#pragma once
#include <glm/glm.hpp>

namespace rfct {
	constexpr float maxWalkVelocity = 0.3f;
	constexpr float dashFullTime = 0.2f;
	struct frameContext;
	void onCollision_Player_StaticObj(entity player, entity collidedWith, glm::vec2 resolution);
	class playerController {
	public:
		playerController();
		void setPlayer(entity playerEntity) { player = playerEntity; }
		void update(frameContext* ctx);
	private:
		entity player;
		float walkSpeed;
		float dashSpeed;
		float jumpSpeed;
		float timeSinceLastDash;

		float walkHorizontalInput;
		float jumpInput;
		float dashHorizontalInput;
		float dashVerticalInput;
		float dash45upInput;
		float dash45downInput;
		bool facingRight;
		bool anyDash = false; // for simplicity

		bool hold;
		
		float notHoldingTime;

		float timesYNotZero;
		float changingDirectionBoost;
		float walkVelocity;
		glm::vec2 dashVelocity;
		int dashCharges;

		uint8_t kindlingsToSpawnThisDash;
	};
}
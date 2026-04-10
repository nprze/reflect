#pragma once
#include <glm/glm.hpp>
#include "world_p/components.h"
#include "world_p/physics/physics.h"

namespace rfct {
	constexpr float dashFullTime = 0.2f;
	class playerController {
	public:
		static playerController& get();
		playerController();
		entity createPlayer(scene* sc, const glm::vec2& spawnPoint);
		void update(frameContext* ctx);
		void fixedUpdate(frameContext* ctx);
		void postFixedUpdate(frameContext* ctx);
		nearestObject findObjectToHold();
		void normalWalkUpdate();
		void normalJumpUpdate();
		bool checkHold(scene* scen); // returns if holding
		void endHold(scene* sc);
		void startDash(frameContext* ctx);
	public:
		// input
		float walkHorizontalInput;
		float jumpInput;
		float dashHorizontalInput;
		float dashVerticalInput;
		float dash45upInput;
		float dash45downInput;
		float arrowUpDownInput;
		bool anyDash; // for simplicity

		entity player;

		float walkVelocity;
		bool facingRight;

		float timeYNotZero;
		float startedJumpingTime;

		float dashTime;
		glm::vec2 dashVelocity;
		float dashCooldown;
		uint8_t kindlingsToSpawnThisDash;

		bool hold;
		float holdCooldown;
		float holdJumpCooldown;
		float holdingTime;
		nearestObject nearestObjectToHold;

		friend class playerAnimations;
	};
}
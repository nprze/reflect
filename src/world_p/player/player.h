#pragma once
#include <glm/glm.hpp>
#include "world_p/components.h"
#include "world_p/physics/physics.h"

namespace rfct {
	constexpr float maxWalkVelocity = 0.3f;
	constexpr float dashFullTime = 0.2f;
	struct frameContext;
	void onCollision_Player_StaticObj(entity player, entity collidedWith, glm::vec2 resolution);
	class playerController {
		static playerController instance;
	public:
		static playerController& get() { return instance; };
		playerController();
		entity createPlayer(scene* sc, const glm::vec2& spawnPoint);
		void update(frameContext* ctx);
		void endHold(scene* sc);
		entity belowBlock = flecs::entity::null();
	private:
		entity player;
		float walkSpeed;
		float dashSpeed;
		float jumpSpeed;

		float walkHorizontalInput;
		float jumpInput;
		float dashHorizontalInput;
		float dashVerticalInput;
		float dash45upInput;
		float dash45downInput;

		float arrowUpDownInput;

		bool anyDash = false; // for simplicity



		float changingDirectionBoost;
		float walkVelocity;
		bool facingRight;


		float timeYNotZero;
		float startedJumpingTime;

		float dashTime;
		glm::vec2 dashVelocity;
		uint8_t kindlingsToSpawnThisDash;


		bool hold;
		float holdingTime = 0.f;
		nearestObject nearestObjectToHold;

		float dashCooldown;
		float holdCooldown;
		float holdJumpCooldown;
	private:
		// to simplify
		positionComponent* posComp;
		velocityComponent* velComp;
		playerStateComponent* stateComp;
		inputVelocityComponent* inputVelComp;

	private:
		nearestObject findObjectToHold();

		void normalWalkUpdate();
		void normalJumpUpdate();


		bool checkHold(scene* scen); // returns if holding

		void startDash(frameContext* ctx);
		friend class playerAnimations;
	};
}
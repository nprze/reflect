#include "player.h"
#include "input.h"
#include "world_p/components.h"
#include "context.h"

constexpr float maxVelocityX = 0.6f;
rfct::playerController::playerController():
	walkSpeed(.06f),
	jumpSpeed(2.f),
	dashSpeed(100.f),
	timeSinceLastDash(0.f),
	walkHorizontalInput(0),
	jumpInput(0),
	dashHorizontalInput(0),
	dashVerticalInput(0),
	dash45upInput(0),
	dash45downInput(0),
	walkVelocity(0),
	dashVelocity(0.f,0.f),
	changingDirectionBoost(0)

{
}
void rfct::playerController::update(const frameContext* ctx)
{
	// normal update
	playerStateComponent* playerState = player.get_mut<playerStateComponent>();
	walkHorizontalInput = 0;

	// dash
	if (timeSinceLastDash != 0.f) {
		timeSinceLastDash += ctx->dt;
		if (timeSinceLastDash > 1.f) {
			timeSinceLastDash = 0.f;
		}
	}
	if ((input::getInput().dashX || input::getInput().dashY || input::getInput().dash45up || input::getInput().dash45down) && timeSinceLastDash == 0.f) {
		timeSinceLastDash = 0.0001f;
		dashHorizontalInput = 0.f;
		dashVerticalInput = 0.f;
		dash45upInput = 0.f;
		dash45downInput = 0.f;
		if (input::getInput().dashX) {
			dashHorizontalInput = input::getInput().dashX;
		}
		else if (input::getInput().dash45up) {
			dash45upInput = input::getInput().dash45up;
		}
		else if (input::getInput().dash45down) {
			dash45downInput = input::getInput().dash45down;
		}
		else if (input::getInput().dashY) {
			dashVerticalInput = input::getInput().dashY;
		}
	}

	// walk
	if (input::getInput().walk != walkHorizontalInput) {
		changingDirectionBoost = 0.5f;
	}
	if (input::getInput().walk) {
		walkHorizontalInput = input::getInput().walk;
	}

	// jump
	if (input::getInput().jump && playerState->grounded) {
		playerState->grounded = false;
		jumpInput = input::getInput().jump;
	}
	// fixedUpdate
	for (uint32_t i = 0; i < ctx->fixedUpdateTimes; ++i) {
		velocityComponent* pos = player.get_mut<velocityComponent>();
		pos->velocity.y += jumpInput * jumpSpeed;
		jumpInput = 0;
		walkVelocity += walkSpeed * walkHorizontalInput;
		walkVelocity = std::clamp(walkVelocity, -maxVelocityX, maxVelocityX);
		walkVelocity += (changingDirectionBoost * changingDirectionBoost * 10) * walkSpeed * walkHorizontalInput;
		changingDirectionBoost = std::clamp(changingDirectionBoost - fixedDeltaTime, 0.f, 0.5f);

		walkVelocity *= 0.90f;
		pos->velocity.x = walkVelocity;
		/*
		pos->velocity.x += dashVerticalInput * dashSpeed;
		pos->velocity.y += dashHorizontalInput * dashSpeed;*/
	}
}

void rfct::onCollision_Player_StaticObj(entity player, entity collidedWith, glm::vec2 resolution)
{
	positionComponent* pos = player.get_mut<positionComponent>();
	pos->position += resolution;


	velocityComponent* vel = player.get_mut<velocityComponent>();
	if (resolution.x != 0.0f) {
		vel->velocity.x = 0.0f;
	}
	if (resolution.y != 0.0f) {
		vel->velocity.y = 0.0f;
	}
	if (resolution.y > 0)
	{
		player.get_mut<playerStateComponent>()->grounded = true;
	}
}

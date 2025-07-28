#include "player.h"
#include "input.h"
#include "world_p/components.h"
#include "context.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/scene.h"
#include "world_p/physics/physics.h"

constexpr float maxVelocityX = 0.6f;
constexpr float boostPureHorizontalVertical = 1.2f;
rfct::playerController::playerController():
	walkSpeed(.06f),
	jumpSpeed(2.f),
	dashSpeed(7.f),
	timeSinceLastDash(0.f),
	walkHorizontalInput(0),
	jumpInput(0),
	dashHorizontalInput(0),
	dashVerticalInput(0),
	dash45upInput(0),
	dash45downInput(0),
	walkVelocity(0),
	dashVelocity(0.f,0.f),
	changingDirectionBoost(0),
	timesYNotZero(0),
	facingRight(true)

{
}
namespace rfct {
	void drawPlayervelocity(const glm::vec2 velComp, const glm::vec2 posComp, const glm::vec2 offset = {0.f, 0.f}) {
		debugLine* line = debugDraw::requestLines(1);
		line->vertices[0].pos = { posComp + offset, 0 };
		line->vertices[1].pos = { posComp + (velComp) + offset, 0 };
		line->vertices[0].color = { std::clamp((float)(glm::length(velComp) / (maxVelocityX * 2) ), 0.f, 1.f), 0.f, 1.f};
		line->vertices[1].color = { std::clamp((float)(glm::length(velComp) / (maxVelocityX * 2)), 0.f, 1.f), 0.f, 1.f};
	}
	float len(const glm::vec2& vec) {
		return std::sqrt((vec.x * vec.x) + (vec.y * vec.y));
	}
}
void rfct::playerController::update(frameContext* ctx)
{
	// draw last frame velocity
	//drawPlayervelocity(player.get<inputVelocityComponent>()->velocity, player.get<positionComponent>()->position);
	//drawPlayervelocity(dashVelocity, player.get<positionComponent>()->position, {2.f, 2.f});

	if (input::getInput().hold) {
		hold = true;
	}

	// normal update
	playerStateComponent* playerState = player.get_mut<playerStateComponent>();
	walkHorizontalInput = 0;

	if ((input::getInput().dashX || input::getInput().dashY || input::getInput().dash45up || input::getInput().dash45down || input::getInput().dashDefault) && (timeSinceLastDash == 0.f || (timeSinceLastDash > 5.f && dashCharges>0))) {
		timeSinceLastDash = 0.0001f;
		dashCharges--;
		if (input::getInput().dashX) {
			dashHorizontalInput = input::getInput().dashX;
			anyDash = true;
		}
		else if (input::getInput().dash45up) {
			dash45upInput = input::getInput().dash45up;
			anyDash = true;
		}
		else if (input::getInput().dash45down) {
			dash45downInput = input::getInput().dash45down;
			anyDash = true;
		}
		else if (input::getInput().dashY) {
			dashVerticalInput = input::getInput().dashY;
			anyDash = true;
		}
		else if (input::getInput().dashDefault) {
			if (facingRight) dashHorizontalInput = 1;
			else dashHorizontalInput = -1;
			anyDash = true;
				
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
		glm::vec2& inputVel = player.get_mut<inputVelocityComponent>()->velocity;
		inputVel = glm::vec2(0.f, 0.f);

		// find object to hold

		if (hold) {
			notHoldingTime = 0.f;
			if (ctx->scene->getObjectHolder().nearestVineEdgeToPlayerIndex == -1) {
				// proritize vines
				glm::vec3 red = glm::vec3(1.f, 0.f, 0.f);
				ctx->scene->getObjectHolder().vineClosestToPlayer = findTheNearestVineToPlayer(player);
				std::pair<glm::vec2, int> vineEdgePos = getNearestEdgePos((player.get<positionComponent>()->position), ctx->scene->getObjectHolder().vineClosestToPlayer);
				if (len(vineEdgePos.first - player.get<positionComponent>()->position) < 0.8) {
					// close enough to start holding
					ctx->scene->getObjectHolder().vineClosestToPlayer.get_mut<vineStateComponent>()->holdingToThis = true;
					ctx->scene->getObjectHolder().nearestVineEdgeToPlayerIndex = vineEdgePos.second;

					playerState->holding = true;
					player.get_mut<gravityComponent>()->gravityEnabled = false;
					player.get_mut <velocityComponent>()->velocity.y = 0.f;
				}
			}
		}
		else {
			notHoldingTime += fixedDeltaTime;
			if (notHoldingTime > fixedDeltaTime * 10 ) { // will hold on to 1/6 seconds after letting go (input sometimes acts up)
				ctx->scene->getObjectHolder().vineClosestToPlayer.get_mut<vineStateComponent>()->holdingToThis = false;
				ctx->scene->getObjectHolder().nearestVineEdgeToPlayerIndex = -1;

				playerState->holding = false;
				player.get_mut<gravityComponent>()->gravityEnabled = true;
			}
		}

		// dash last time calculate
		if (timeSinceLastDash != 0.f) {
			timeSinceLastDash += fixedDeltaTime;
			if (timeSinceLastDash >= 1.f) {
				timeSinceLastDash = 0.f;
			}
		}

		// calculate if player is midair (forgive 50 ms)
		velocityComponent* pos = player.get_mut<velocityComponent>();
		if (pos->velocity.y != 0) {
			timesYNotZero += fixedDeltaTime;
			if (timesYNotZero > fixedDeltaTime * 3) {
				playerStateComponent* ps = player.get_mut<playerStateComponent>();
				ps->grounded = false;
			}
		}
		else {
			timesYNotZero = 0;
			dashCharges = 1;
		}

		// calculate direction
		if (pos->velocity.x > 0) {
			facingRight = true;
		}
		else if (pos->velocity.x<0) {
			facingRight = false;
		}

		// jump apply
		pos->velocity.y += jumpInput * jumpSpeed;
		inputVel.y += jumpInput * jumpSpeed;

		walkVelocity += walkSpeed * walkHorizontalInput;
		walkVelocity = std::clamp(walkVelocity, -maxVelocityX, maxVelocityX);
		walkVelocity += (changingDirectionBoost * changingDirectionBoost * 10) * walkSpeed * walkHorizontalInput;
		changingDirectionBoost = std::clamp(changingDirectionBoost - fixedDeltaTime, 0.f, 0.5f);


		// walk apply
		walkVelocity *= 0.80f;
		inputVel.x += walkVelocity;
		pos->velocity.x = walkVelocity;

		// dash apply
		if (anyDash) {
			player.get_mut<velocityComponent>()->velocity = { 0.f,0.f };
			dashVelocity = { 0, 0 };
			dashVelocity.y += dashVerticalInput * dashSpeed * boostPureHorizontalVertical;
			dashVelocity.x += dashHorizontalInput * dashSpeed * boostPureHorizontalVertical;
			dashVelocity.x += (dash45upInput + dash45downInput) * ((float)1 / std::sqrt(2)) * dashSpeed;
			dashVelocity.y += (dash45upInput - dash45downInput) * ((float)1 / std::sqrt(2)) * dashSpeed;
		}
		if (timeSinceLastDash != 0 && timeSinceLastDash<0.2f) {
			player.get_mut<gravityComponent>()->gravityEnabled = false;
		}
		else {
			if (!player.get<playerStateComponent>()->holding) {
				player.get_mut<gravityComponent>()->gravityEnabled = true;
			}
		}
		dashVelocity *= 0.9f - std::clamp(timeSinceLastDash * 2.f, 0.f, 0.9f);
		if (glm::length(dashVelocity) >= 0.1f) {
			pos->velocity = dashVelocity;
			inputVel = dashVelocity;
		}

		// reset after applying
		jumpInput = 0;
		dashHorizontalInput = 0.f;
		dashVerticalInput = 0.f;
		dash45upInput = 0.f;
		dash45downInput = 0.f;
		anyDash = false;
		hold = false;
	}
	ctx->scene->updateDirection(facingRight);
}

void rfct::playerController::startHoldingToVine()
{
}

void rfct::playerController::endHoldingToVine()
{
}

void rfct::onCollision_Player_StaticObj(entity player, entity collidedWith, glm::vec2 resolution)
{
	positionComponent* pos = player.get_mut<positionComponent>();
	pos->position += resolution;


	velocityComponent* vel = player.get_mut<velocityComponent>();
	inputVelocityComponent* ivel = player.get_mut<inputVelocityComponent>();
	if (resolution.x != 0.0f) {
		vel->velocity.x = 0.0f;
		ivel->velocity.x = 0.0f;
	}
	if (resolution.y != 0.0f) {
		vel->velocity.y = 0.0f;
		ivel->velocity.y = 0.0f;
	}
	if (resolution.y > 0)
	{
		player.get_mut<playerStateComponent>()->grounded = true;
	}
}

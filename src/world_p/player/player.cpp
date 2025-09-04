#include "player.h"
#include "input.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "context.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/scene.h"
#include "world_p/physics/physics.h"
#include "world_p/decors/dash_kindlings.h"

constexpr float maxVelocityX = 0.6f;
constexpr float boostPureHorizontalVertical = 1.2f;

rfct::playerController rfct::playerController::instance;

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
	struct nearestObject {
		entity object;
		glm::vec2 closestPosition;
		int vineIndex; // -2 if block, -1 if no object found
	};
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
	RFCT_PROFILE_SCOPE("player update");
	if (player.get<playerBoxToHoldComponent>()->boundingBox) {
		glm::vec2 nearestPoint = nearestPointOnAABB(player.get<positionComponent>()->position, player.get<playerBoxToHoldComponent>()->boundingBox->min, player.get<playerBoxToHoldComponent>()->boundingBox->max);
		drawAABB(player.get<playerBoxToHoldComponent>()->boundingBox->min, player.get<playerBoxToHoldComponent>()->boundingBox->max, 0);
		drawAABB(nearestPoint - glm::vec2{0.5f, 0.5f}, nearestPoint + glm::vec2{ 0.5f, 0.5f }, 1);
	}
	// draw last frame velocity
	//drawPlayervelocity(player.get<inputVelocityComponent>()->velocity, player.get<positionComponent>()->position);
	//drawPlayervelocity(player.get<velocityComponent>()->velocity, player.get<positionComponent>()->position);

	if (input::getInput().hold) {
		hold = true;
	}

	// normal update
	playerStateComponent* state = player.get_mut<playerStateComponent>();
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
	if (input::getInput().jump && state->grounded) {
		state->grounded = false;
		jumpInput = input::getInput().jump;
	}

	

	// fixedUpdate
	for (uint32_t i = 0; i < ctx->fixedUpdateTimes; ++i) {

		glm::vec2& inputVel = player.get_mut<inputVelocityComponent>()->velocity;
		inputVel = glm::vec2(0.f, 0.f);

		positionComponent* posComp = player.get_mut<positionComponent>();
		velocityComponent* velComp = player.get_mut<velocityComponent>();
		playerStateComponent* stateComp = player.get_mut<playerStateComponent>();


		// returns object entity and if that entity is vine
		auto findObjectToHold = [&]() {
			constexpr float forgivenessVine = 0.8f;
			nearestObject returnVal;
			// proritize vines
			returnVal.object = findTheNearestVineToPlayer(player);
			std::pair<glm::vec2, int> vineEdgePos = getNearestEdgePos((player.get<positionComponent>()->position), returnVal.object);
			returnVal.closestPosition = vineEdgePos.first;
			if (len(returnVal.closestPosition - posComp->position) < forgivenessVine) {
				returnVal.vineIndex = vineEdgePos.second;
				return returnVal;

				// close enough to start holding
				ctx->scene->getObjectHolder().vineClosestToPlayer.get_mut<vineStateComponent>()->holdingToThis = true;
				ctx->scene->getObjectHolder().nearestVineEdgeToPlayerIndex = vineEdgePos.second;

				stateComp->holding = true;
				player.get_mut<gravityComponent>()->gravityEnabled = false;
				player.get_mut <velocityComponent>()->velocity.y = 0.f;
			}
			// vine is too far, fallback to block
			returnVal.object = findTheNearestBlockToPlayer(player);
			const staticBoxColliderComponent* boc = returnVal.object.get<staticBoxColliderComponent>();
			returnVal.closestPosition = nearestPointOnAABB(posComp->position, boc->min, boc->max);
			if (len(returnVal.closestPosition - posComp->position) < (forgivenessVine * 0.5f)) {
				returnVal.vineIndex = -2;
				return returnVal;
				stateComp->holding = true;
				player.get_mut<gravityComponent>()->gravityEnabled = false;
				player.get_mut<velocityComponent>()->velocity.y = 0.f;
			}
			// all objects too far
			returnVal.vineIndex = -1;
			returnVal.closestPosition = { 0,0 };
			returnVal.object = entity();
			return returnVal;
			};

		auto normalWalkUpdate = [&]() {
			walkVelocity += walkSpeed * walkHorizontalInput;
			walkVelocity = std::clamp(walkVelocity, -maxVelocityX, maxVelocityX);
			walkVelocity += (changingDirectionBoost * changingDirectionBoost * 10) * walkSpeed * walkHorizontalInput;
			changingDirectionBoost = std::clamp(changingDirectionBoost - fixedDeltaTime, 0.f, 0.5f);


			// walk apply
			walkVelocity *= 0.80f;
			inputVel.x += walkVelocity;
			velComp->velocity.x = walkVelocity;
			};

		auto normalJumpUpdate = [&]() {
			velComp->velocity.y += jumpInput * jumpSpeed;
			inputVel.y += jumpInput * jumpSpeed;
			};

		auto normalHoldUpdate = [&]() {
			if (hold) {
				if (!player.get<playerBoxToHoldComponent>()->boundingBox) {
					nearestObject nearest = findObjectToHold();
					if (nearest.vineIndex == -2) {
						// a box
						player.get_mut< playerBoxToHoldComponent>()->boundingBox = nearest.object.get_mut<staticBoxColliderComponent>();
					}
				}
			}
			else {
				notHoldingTime += fixedDeltaTime;
				if (notHoldingTime > fixedDeltaTime * 2 && stateComp->holding) {
					ctx->scene->getObjectHolder().vineClosestToPlayer.get_mut<vineStateComponent>()->holdingToThis = false;
					ctx->scene->getObjectHolder().nearestVineEdgeToPlayerIndex = -1;

					stateComp->holding = false;
					player.get_mut<gravityComponent>()->gravityEnabled = true;

					glm::vec2 launchDir = glm::vec2(0.f);

					launchDir.x = facingRight ? 1.f : -1.f;

					launchDir.y = 0.2f;
					player.get_mut<velocityComponent>()->velocity += launchDir * 10.f;
				}
			}
			};




		switch (stateComp->state)
		{
		case (playerState::normal): {

			normalWalkUpdate();
			normalJumpUpdate();
			normalHoldUpdate();
			if (jumpInput != 0) {
				//stateComp->state = playerState::jumping;
			}
			if (hold == 0) {

			}

			break;
		}
		case (playerState::dashing): {
			break;
		}
		case (playerState::jumping): {
			normalWalkUpdate();
			break;
		}
		case (playerState::holdingVines): {
			break;
		}
		case (playerState::holdingBlocks): {
			break;
		}
		default:
			break;
		}
		// find object to hold
		/*
		if (hold) {
			notHoldingTime = 0.f;
			if (!stateComp->holding) {
			}
		}
		else {
			notHoldingTime += fixedDeltaTime;
			if (notHoldingTime > fixedDeltaTime * 2 && stateComp->holding) {
				ctx->scene->getObjectHolder().vineClosestToPlayer.get_mut<vineStateComponent>()->holdingToThis = false;
				ctx->scene->getObjectHolder().nearestVineEdgeToPlayerIndex = -1;

				stateComp->holding = false;
				player.get_mut<gravityComponent>()->gravityEnabled = true;

				glm::vec2 launchDir = glm::vec2(0.f);

				launchDir.x = facingRight ? 1.f : -1.f;

				launchDir.y = 0.2f;
				player.get_mut<velocityComponent>()->velocity += launchDir * 10.f;
			}
		}*/

		// dash last time calculate
		if (timeSinceLastDash != 0.f) {
			timeSinceLastDash += fixedDeltaTime;
			if (timeSinceLastDash >= 1.f) {
				timeSinceLastDash = 0.f;
			}
		}

		// calculate if player is midair (forgive ~50 ms)
		if (velComp->velocity.y != 0) {
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
		if (velComp->velocity.x > 0) {
			facingRight = true;
		}
		else if (velComp->velocity.x<0) {
			facingRight = false;
		}

		// dash apply
		if (anyDash) {
			player.get_mut<velocityComponent>()->velocity = { 0.f,0.f };
			dashVelocity = { 0, 0 };
			dashVelocity.y += dashVerticalInput * dashSpeed * boostPureHorizontalVertical;
			dashVelocity.x += dashHorizontalInput * dashSpeed * boostPureHorizontalVertical;
			dashVelocity.x += (dash45upInput + dash45downInput) * ((float)1 / std::sqrt(2)) * dashSpeed;
			dashVelocity.y += (dash45upInput - dash45downInput) * ((float)1 / std::sqrt(2)) * dashSpeed;

			ctx->scene->getObjectHolder().onPlayerDashObjects(ctx, player, facingRight);
			ctx->scene->getDecorationHolder().onPlayerDashDecorations(ctx, player, facingRight);
			kindlingsToSpawnThisDash = 3;
		}
		if (timeSinceLastDash != 0 && timeSinceLastDash<dashFullTime) {
			playerDashStateComponent* dc = player.get_mut<playerDashStateComponent>();
			player.get_mut<gravityComponent>()->gravityEnabled = false;
			dc->dashing = true;
			dc->dashProgress = timeSinceLastDash / dashFullTime;

			if (dc->dashProgress > 0.1f && kindlingsToSpawnThisDash == 3) {
				kindlingsToSpawnThisDash -= 1;
				spawnKindling(ctx, player.get_mut<positionComponent>()->position, velComp->velocity, kindlingsToSpawnThisDash);
			}
			else {
				if (dc->dashProgress > 0.3f && kindlingsToSpawnThisDash == 2) {
					kindlingsToSpawnThisDash -= 1;
					spawnKindling(ctx, player.get_mut<positionComponent>()->position, velComp->velocity, kindlingsToSpawnThisDash);
				}
				else {
					if (dc->dashProgress > 0.9f && kindlingsToSpawnThisDash == 1) {
						kindlingsToSpawnThisDash -= 1;
						spawnKindling(ctx, player.get_mut<positionComponent>()->position, velComp->velocity, kindlingsToSpawnThisDash);
					}
				}

			}
		}
		else {
			player.get_mut<playerDashStateComponent>()->dashing = false;
			player.get_mut<playerDashStateComponent>()->dashProgress = 0.f;
			if (!player.get<playerStateComponent>()->holding) {
				player.get_mut<gravityComponent>()->gravityEnabled = true;
			}
		}
		dashVelocity *= 0.9f - std::clamp(timeSinceLastDash * 2.f, 0.f, 0.9f);
		if (glm::length(dashVelocity) >= 0.1f) {
			velComp->velocity = dashVelocity;
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
		if (resolution.y > 0.0f) {
			// Landed on something
			vel->velocity.y = 0.0f;
			ivel->velocity.y = 0.0f;
		}
		else {
			if (resolution.x != 0.f) {
				// Hit your head on a ceiling: only stop upward motion
				if (vel->velocity.y > 0.0f) {
					vel->velocity.y = 0.0f;
					ivel->velocity.y = 0.0f;
				}
			}
		}
	}
	if (resolution.y > 0)
	{
		player.get_mut<playerStateComponent>()->grounded = true;
	}
}

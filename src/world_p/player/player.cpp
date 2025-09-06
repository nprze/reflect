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

rfct::playerController::playerController() :
	walkSpeed(.06f),
	jumpSpeed(1.3f),
	dashSpeed(7.f),
	walkHorizontalInput(0),
	jumpInput(0),
	dashHorizontalInput(0),
	dashVerticalInput(0),
	dash45upInput(0),
	dash45downInput(0),
	walkVelocity(0),
	dashVelocity(0.f, 0.f),
	changingDirectionBoost(0),
	timeYNotZero(0),
	facingRight(true),
	nearestObjectToHold(),
	startedJumpingTime(0.f)
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
	RFCT_PROFILE_SCOPE("player update");
	
	{
		// draw box
		const positionComponent* pos = player.get<positionComponent>();
		const dynamicBoxColliderComponent* boc = player.get<dynamicBoxColliderComponent>();
		debugLine* lines = debugDraw::requestLines(4);
		lines[0].vertices[0].pos = { pos->position.x+boc->min.x, pos->position.y + boc->min.y, 0 };
		lines[0].vertices[1].pos = { pos->position.x+boc->min.x, pos->position.y + boc->max.y, 0 };

		lines[1].vertices[0].pos = { pos->position.x+boc->min.x, pos->position.y + boc->max.y, 0 };
		lines[1].vertices[1].pos = { pos->position.x+boc->max.x, pos->position.y + boc->max.y, 0 };

		lines[2].vertices[0].pos = { pos->position.x+boc->max.x, pos->position.y + boc->max.y, 0 };
		lines[2].vertices[1].pos = { pos->position.x+boc->max.x, pos->position.y + boc->min.y, 0 };

		lines[3].vertices[0].pos = { pos->position.x+boc->max.x, pos->position.y + boc->min.y, 0 };
		lines[3].vertices[1].pos = { pos->position.x+boc->min.x, pos->position.y + boc->min.y, 0 };

		for (uint8_t i = 0; i < 4; i++) {
			lines[i].vertices[0].color = {0.8f, 0.8f, 0.8f};
			lines[i].vertices[1].color = {0.8f, 0.8f, 0.8f};
		}

		// draw circle
		const int segments = 20;
		const dynamicCircleColliderComponent* circ = player.get<dynamicCircleColliderComponent>();
		lines = debugDraw::requestLines(segments);

		const float step = 2.0f * 3.14159265359f / segments;

		for (int i = 0; i < segments; i++) {
			float angle1 = i * step;
			float angle2 = (i + 1) * step;

			float x1 = pos->position.x + circ->offsetFromCenter.x + circ->radius * cosf(angle1);
			float y1 = pos->position.y + circ->offsetFromCenter.y + circ->radius * sinf(angle1);

			float x2 = pos->position.x + circ->offsetFromCenter.x + circ->radius * cosf(angle2);
			float y2 = pos->position.y + circ->offsetFromCenter.y + circ->radius * sinf(angle2);

			lines[i].vertices[0].pos = { x1, y1, 0.0f };
			lines[i].vertices[1].pos = { x2, y2, 0.0f };

			lines[i].vertices[0].color = { 0.8f, 0.4f, 0.4f };
			lines[i].vertices[1].color = { 0.8f, 0.4f, 0.4f };
		}
	}
	if (nearestObjectToHold.vineIndex == -2) {
		const staticBoxColliderComponent* col = nearestObjectToHold.object.get<staticBoxColliderComponent>();
		drawAABB(col->min, col->max, 0);
		drawAABB(nearestObjectToHold.closestPosition - glm::vec2{0.5f, 0.5f}, nearestObjectToHold.closestPosition + glm::vec2{ 0.5f, 0.5f }, 1);
	}
	if (nearestObjectToHold.vineIndex >= 0) {
		drawAABB(nearestObjectToHold.closestPosition - glm::vec2{0.5f, 0.5f}, nearestObjectToHold.closestPosition + glm::vec2{ 0.5f, 0.5f }, 1);
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

	if ((input::getInput().dashX || input::getInput().dashY || input::getInput().dash45up || input::getInput().dash45down || input::getInput().dashDefault) && dashCharges>0) {
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
	if (input::getInput().jump && player.get<playerStateComponent>()->allowToJump) {
		jumpInput = input::getInput().jump;
	}

	

	// fixedUpdate
	for (uint32_t i = 0; i < ctx->fixedUpdateTimes; ++i) {
		inputVelComp = player.get_mut<inputVelocityComponent>();
		glm::vec2& inputVel = inputVelComp->velocity;
		inputVel = glm::vec2(0.f, 0.f);

		posComp = player.get_mut<positionComponent>();
		velComp = player.get_mut<velocityComponent>();
		stateComp = player.get_mut<playerStateComponent>();


		switch (stateComp->state)
		{
		case (playerState::normal): {
			if (velComp->velocity.y == 0) {
				timeYNotZero = 0;
				stateComp->allowToJump = true;
				dashCharges = 1;
			}
			else {
				timeYNotZero += fixedDeltaTime;
				if (timeYNotZero > fixedDeltaTime * 3) {
					stateComp->allowToJump = false;
				}
			}

			normalWalkUpdate();
			if (jumpInput != 0) {
				startedJumpingTime = 0.f;
				stateComp->state = playerState::jumping;
			}
			if (anyDash) {
				dashTime = 0.f;
				stateComp->state = playerState::dashing;
			}
			break;
		}
		case (playerState::dashing): {
			if (dashTime == 0.f) {
				// start das
				startDash(ctx);
			}
			dashTime += fixedDeltaTime;
			if (dashTime <= dashFullTime) {
				// update dash
				dashVelocity *= 0.9f - std::clamp(dashTime * 2.f, 0.f, 0.9f);
				if (glm::length(dashVelocity) >= 0.1f) {
					velComp->velocity = dashVelocity;
					inputVel = dashVelocity;
				}


				// update kindlings
				playerDashStateComponent* dc = player.get_mut<playerDashStateComponent>();
				player.get_mut<gravityComponent>()->gravityEnabled = false;
				dc->dashing = true;
				dc->dashProgress = dashTime / dashFullTime;

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
				// end dash
				dashTime = 0.f;
				player.get_mut<gravityComponent>()->gravityEnabled = true;
				stateComp->state = playerState::normal;
			}
			break;
		}
		case (playerState::jumping): {
			normalWalkUpdate();
			normalJumpUpdate();
			if (velComp->velocity.y == 0.f) {
				stateComp->allowToJump = true;
				stateComp->state = playerState::normal;
			}
			if (anyDash) {
				dashTime = 0.f;
				stateComp->state = playerState::dashing;
			}
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

/*
		//// find object to hold
		///*
		//if (hold) {
		//	notHoldingTime = 0.f;
		//	if (!stateComp->holding) {
		//	}
		//}
		//else {
		//	notHoldingTime += fixedDeltaTime;
		//	if (notHoldingTime > fixedDeltaTime * 2 && stateComp->holding) {
		//		ctx->scene->getObjectHolder().vineClosestToPlayer.get_mut<vineStateComponent>()->holdingToThis = false;
		//		ctx->scene->getObjectHolder().nearestVineEdgeToPlayerIndex = -1;

		//		stateComp->holding = false;
		//		player.get_mut<gravityComponent>()->gravityEnabled = true;

		//		glm::vec2 launchDir = glm::vec2(0.f);

		//		launchDir.x = facingRight ? 1.f : -1.f;

		//		launchDir.y = 0.2f;
		//		player.get_mut<velocityComponent>()->velocity += launchDir * 10.f;
		//	}
		//}

		//// dash last time calculate
		//if (timeSinceLastDash != 0.f) {
		//	timeSinceLastDash += fixedDeltaTime;
		//	if (timeSinceLastDash >= 1.f) {
		//		timeSinceLastDash = 0.f;
		//	}
		//}


		//// calculate direction
		//if (velComp->velocity.x > 0) {
		//	facingRight = true;
		//}
		//else if (velComp->velocity.x<0) {
		//	facingRight = false;
		//}

		//// dash apply
		//if (anyDash) {
		//	player.get_mut<velocityComponent>()->velocity = { 0.f,0.f };
		//	dashVelocity = { 0, 0 };
		//	dashVelocity.y += dashVerticalInput * dashSpeed * boostPureHorizontalVertical;
		//	dashVelocity.x += dashHorizontalInput * dashSpeed * boostPureHorizontalVertical;
		//	dashVelocity.x += (dash45upInput + dash45downInput) * ((float)1 / std::sqrt(2)) * dashSpeed;
		//	dashVelocity.y += (dash45upInput - dash45downInput) * ((float)1 / std::sqrt(2)) * dashSpeed;

		//	ctx->scene->getObjectHolder().onPlayerDashObjects(ctx, player, facingRight);
		//	ctx->scene->getDecorationHolder().onPlayerDashDecorations(ctx, player, facingRight);
		//	kindlingsToSpawnThisDash = 3;
		//}
		//if (timeSinceLastDash != 0 && timeSinceLastDash<dashFullTime) {
		//	playerDashStateComponent* dc = player.get_mut<playerDashStateComponent>();
		//	player.get_mut<gravityComponent>()->gravityEnabled = false;
		//	dc->dashing = true;
		//	dc->dashProgress = timeSinceLastDash / dashFullTime;

		//	if (dc->dashProgress > 0.1f && kindlingsToSpawnThisDash == 3) {
		//		kindlingsToSpawnThisDash -= 1;
		//		spawnKindling(ctx, player.get_mut<positionComponent>()->position, velComp->velocity, kindlingsToSpawnThisDash);
		//	}
		//	else {
		//		if (dc->dashProgress > 0.3f && kindlingsToSpawnThisDash == 2) {
		//			kindlingsToSpawnThisDash -= 1;
		//			spawnKindling(ctx, player.get_mut<positionComponent>()->position, velComp->velocity, kindlingsToSpawnThisDash);
		//		}
		//		else {
		//			if (dc->dashProgress > 0.9f && kindlingsToSpawnThisDash == 1) {
		//				kindlingsToSpawnThisDash -= 1;
		//				spawnKindling(ctx, player.get_mut<positionComponent>()->position, velComp->velocity, kindlingsToSpawnThisDash);
		//			}
		//		}

		//	}
		//}
		//else {
		//	player.get_mut<playerDashStateComponent>()->dashing = false;
		//	player.get_mut<playerDashStateComponent>()->dashProgress = 0.f;
		//	
		//	if (!player.get<playerStateComponent>()->holding) {
		//		player.get_mut<gravityComponent>()->gravityEnabled = true;
		//	}
		//}
		//dashVelocity *= 0.9f - std::clamp(timeSinceLastDash * 2.f, 0.f, 0.9f);
		//if (glm::length(dashVelocity) >= 0.1f) {
		//	velComp->velocity = dashVelocity;
		//	inputVel = dashVelocity;
		//}
*/

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

rfct::nearestObject rfct::playerController::findObjectToHold()
{
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
		//ctx->scene->getObjectHolder().vineClosestToPlayer.get_mut<vineStateComponent>()->holdingToThis = true;
		//ctx->scene->getObjectHolder().nearestVineEdgeToPlayerIndex = vineEdgePos.second;

		//stateComp->holding = true;
		player.get_mut<gravityComponent>()->gravityEnabled = false;
		player.get_mut <velocityComponent>()->velocity.y = 0.f;
	}
	// vine is too far, fallback to block
	returnVal.object = findTheNearestBlockToPlayer(player);
	const staticBoxColliderComponent* boc = returnVal.object.get<staticBoxColliderComponent>();
	returnVal.closestPosition = nearestPointOnAABB(posComp->position, boc->min, boc->max);
	if (len(returnVal.closestPosition - posComp->position) < (forgivenessVine)) {
		returnVal.vineIndex = -2;
		return returnVal;
		//stateComp->holding = true;
		player.get_mut<gravityComponent>()->gravityEnabled = false;
		player.get_mut<velocityComponent>()->velocity.y = 0.f;
	}
	// all objects too far
	returnVal.vineIndex = -1;
	returnVal.closestPosition = { 0,0 };
	returnVal.object = entity();
	return returnVal;
}

void rfct::playerController::normalWalkUpdate()
{
	walkVelocity += walkSpeed * walkHorizontalInput;
	walkVelocity = std::clamp(walkVelocity, -maxVelocityX, maxVelocityX);
	walkVelocity += (changingDirectionBoost * changingDirectionBoost * 10) * walkSpeed * walkHorizontalInput;
	changingDirectionBoost = std::clamp(changingDirectionBoost - fixedDeltaTime, 0.f, 0.5f);


	// walk apply
	walkVelocity *= 0.80f;
	inputVelComp->velocity.x += walkVelocity;
	velComp->velocity.x = walkVelocity;
}

void rfct::playerController::normalJumpUpdate()
{
	if (startedJumpingTime == 0.f) {
		// first 
		startedJumpingTime = 0.01f;
	}
	startedJumpingTime += fixedDeltaTime;
	float inputMultiplayer = std::clamp(- std::sqrt(7.f * startedJumpingTime) + 1.f, 0.f, 1.f);
	velComp->velocity.y += inputMultiplayer * jumpInput * jumpSpeed;
	inputVelComp->velocity.y += inputMultiplayer * jumpInput * jumpSpeed;
}

void rfct::playerController::normalDashUpdate()
{
}

void rfct::playerController::normalHoldUpdate()
{
	if (hold) {
		RFCT_INFO("starting hold");
		nearestObjectToHold = findObjectToHold();
		if (nearestObjectToHold.vineIndex == -2) {
			RFCT_INFO("found box to hold");
			stateComp->state = playerState::holdingBlocks;
		}

	}
	else {
		/*
		notHoldingTime += fixedDeltaTime;
		if (notHoldingTime > fixedDeltaTime * 2) {
			RFCT_INFO("ending hold");
			nearestObjectToHold = nearestObject();

			stateComp->holding = false;
			player.get_mut<gravityComponent>()->gravityEnabled = true;

			glm::vec2 launchDir = glm::vec2(0.f);

			launchDir.x = facingRight ? 1.f : -1.f;

			launchDir.y = 0.2f;
			player.get_mut<velocityComponent>()->velocity += launchDir * 10.f;
		}*/
	}
}

void rfct::playerController::startDash(frameContext* ctx)
{
	dashCharges--;
	player.get_mut<velocityComponent>()->velocity = { 0.f,0.f };
	dashVelocity = { 0, 0 };
	dashVelocity.y += dashVerticalInput * dashSpeed * boostPureHorizontalVertical;
	dashVelocity.x += dashHorizontalInput * dashSpeed * boostPureHorizontalVertical;
	dashVelocity.x += (dash45upInput + dash45downInput) * ((float)1 / std::sqrt(2)) * dashSpeed;
	dashVelocity.y += (dash45upInput - dash45downInput) * ((float)1 / std::sqrt(2)) * dashSpeed;

	ctx->scene->getObjectHolder().onPlayerDashObjects(ctx, player, facingRight);
	ctx->scene->getDecorationHolder().onPlayerDashDecorations(ctx, player, facingRight);
	kindlingsToSpawnThisDash = 3;
	stateComp->allowToJump = false;
	player.get_mut<gravityComponent>()->gravityEnabled = false;
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

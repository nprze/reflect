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
	jumpSpeed(1.1f),
	dashSpeed(8.f),
	arrowUpDownInput(0),
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
	startedJumpingTime(0.f),
	dashCooldown(0.f)
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

	// debug helpers
	// colliders
	if (false) {
		// draw box
		const positionComponent* pos = player.get<positionComponent>();
		const dynamicBoxColliderComponent* boc = player.get<dynamicBoxColliderComponent>();
		debugLine* lines = debugDraw::requestLines(4);
		lines[0].vertices[0].pos = { pos->position.x + boc->min.x, pos->position.y + boc->min.y, 0 };
		lines[0].vertices[1].pos = { pos->position.x + boc->min.x, pos->position.y + boc->max.y, 0 };

		lines[1].vertices[0].pos = { pos->position.x + boc->min.x, pos->position.y + boc->max.y, 0 };
		lines[1].vertices[1].pos = { pos->position.x + boc->max.x, pos->position.y + boc->max.y, 0 };

		lines[2].vertices[0].pos = { pos->position.x + boc->max.x, pos->position.y + boc->max.y, 0 };
		lines[2].vertices[1].pos = { pos->position.x + boc->max.x, pos->position.y + boc->min.y, 0 };

		lines[3].vertices[0].pos = { pos->position.x + boc->max.x, pos->position.y + boc->min.y, 0 };
		lines[3].vertices[1].pos = { pos->position.x + boc->min.x, pos->position.y + boc->min.y, 0 };

		for (uint8_t i = 0; i < 4; i++) {
			lines[i].vertices[0].color = { 0.8f, 0.8f, 0.8f };
			lines[i].vertices[1].color = { 0.8f, 0.8f, 0.8f };
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
	// object to hold
	if (false){
		if (nearestObjectToHold.vineIndex == -2) {
			const staticBoxColliderComponent* col = nearestObjectToHold.object.get<staticBoxColliderComponent>();
			drawAABB(col->min, col->max, 0);
			debugLine* lines = debugDraw::requestLines(2);
			lines[0].vertices[0].pos = { nearestObjectToHold.closestPosition - glm::vec2{0, 0.5f}, 0 };
			lines[0].vertices[1].pos = { nearestObjectToHold.closestPosition + glm::vec2{0, 0.5f}, 0 };

			lines[1].vertices[0].pos = { nearestObjectToHold.closestPosition - glm::vec2{0.5f, 0}, 0 };
			lines[1].vertices[1].pos = { nearestObjectToHold.closestPosition + glm::vec2{0.5f, 0}, 0 };

			for (uint8_t i = 0; i < 2; i++) {
				lines[i].vertices[0].color = { 0.2f, 0.2f, 0.8f };
				lines[i].vertices[1].color = { 0.2f, 0.2f, 0.8f };
			}

			
		}
		if (nearestObjectToHold.vineIndex >= 0) {
			const positionComponent* pos = player.get<positionComponent>();
			debugLine* lines = debugDraw::requestLines(2);
			lines[0].vertices[0].pos = { nearestObjectToHold.closestPosition - glm::vec2{0, 50}, 0 };
			lines[0].vertices[1].pos = { nearestObjectToHold.closestPosition + glm::vec2{0, 50}, 0 };

			lines[1].vertices[0].pos = { nearestObjectToHold.closestPosition - glm::vec2{50, 0}, 0 };
			lines[1].vertices[1].pos = { nearestObjectToHold.closestPosition + glm::vec2{50, 0}, 0 };

			for (uint8_t i = 0; i < 2; i++) {
				lines[i].vertices[0].color = { 0.8f, 0.8f, 0.8f };
				lines[i].vertices[1].color = { 0.8f, 0.8f, 0.8f };
			}
		}
	}
	// last frame velocity
	if (false) {
		drawPlayervelocity(player.get<inputVelocityComponent>()->velocity, player.get<positionComponent>()->position);
		drawPlayervelocity(player.get<velocityComponent>()->velocity, player.get<positionComponent>()->position);
	}


	if (input::getInput().hold) {
		hold = true;
	}

	// normal update
	playerStateComponent* state = player.get_mut<playerStateComponent>();
	walkHorizontalInput = 0;

	arrowUpDownInput = input::getInput().upDown;

	if ((input::getInput().dashX || input::getInput().dashY || input::getInput().dash45up || input::getInput().dash45down || input::getInput().dashDefault) && dashCharges>0 && dashCooldown <= 0.f) {
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

		dashCooldown = std::clamp(dashCooldown - fixedDeltaTime, 0.f, 3.f);
		holdCooldown = std::clamp(holdCooldown - fixedDeltaTime, 0.f, 3.f);
		holdJumpCooldown = std::clamp(holdJumpCooldown - fixedDeltaTime, 0.f, .5f);

		switch (stateComp->state)
		{
		case (playerState::normal): {
			if (velComp->velocity.y == 0) {
				timeYNotZero = 0;
				stateComp->allowToJump = true;
				if (dashCooldown == 0.f) {
					dashCharges = 1;
				}
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
			if (!checkHold(ctx->scene)) {
				if (anyDash) {
					dashTime = 0.f;
					stateComp->state = playerState::dashing;
				}
			}
			break;
		}
		case (playerState::dashing): {
			if (dashTime == 0.f) {
				startDash(ctx);
			}
			normalWalkUpdate();
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
				dashCooldown = .2f;
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

			// hold priority over dash
			if (!checkHold(ctx->scene)) {
				if (anyDash) {
					dashTime = 0.f;
					stateComp->state = playerState::dashing;
				}
			}
			break;
		}
		case (playerState::holdingVines): {
			normalWalkUpdate();
			stateComp->allowToJump = false;
			player.get_mut<gravityComponent>()->gravityEnabled = false;
			if (!hold) {
				stateComp->state = playerState::normal;
			}
			if (anyDash) {
				dashTime = 0.f;
				stateComp->state = playerState::dashing;
			}

			if (stateComp->state != playerState::holdingVines) {
				endHold(ctx->scene);
			}
			break;
		}
		case (playerState::holdingBlocks): {
			nearestObjectToHold = findObjectToHold();
			if (
				(nearestObjectToHold.closestPosition.y < posComp->position.y) || (nearestObjectToHold.closestPosition.y > posComp->position.y)) {
				stateComp->state = playerState::normal;
				posComp->position.y += std::abs(nearestObjectToHold.closestPosition.x - posComp->position.x) * 0.9f;
			}
			if (!hold) {
				stateComp->state = playerState::normal;
			}
			stateComp->allowToJump = true;
			player.get_mut<gravityComponent>()->gravityEnabled = false;
			if (jumpInput != 0 && holdJumpCooldown == 0.f) {
				startedJumpingTime = 0.f;
				stateComp->state = playerState::jumping;
			}

			float move = arrowUpDownInput;
			move = std::clamp(move, -maxVelocityX * 0.75f, maxVelocityX * 0.75f);
			velComp->velocity.y = move;
			velComp->velocity.x = 0;

			if (anyDash) {
				dashTime = 0.f;
				stateComp->state = playerState::dashing;
			}

			if (stateComp->state != playerState::holdingBlocks) {
				if (stateComp->state == playerState::normal) {
					player.get_mut<positionComponent>()->position.x += (facingRight ? 1.f : -1.f) * 0.3f;
				}
				player.get_mut<gravityComponent>()->gravityEnabled = true;
				holdCooldown = 0.25f;
				//velComp->velocity.y += .5f;
				holdJumpCooldown = 0.5f;
			}
			break;
		}
		default:
			break;
		}
		 
		if (velComp->velocity.x > 0) {
			facingRight = true;
		}
		else if (velComp->velocity.x<0) {
			facingRight = false;
		}

		// trigger only once
		anyDash = false;
		dashHorizontalInput = 0.f;
		dashVerticalInput = 0.f;
		dash45upInput = 0.f;
		dash45downInput = 0.f;
		arrowUpDownInput = 0.f;
	}
	// reset after applying
	jumpInput = 0;
	hold = false;
	ctx->scene->updateDirection(facingRight);
}

rfct::nearestObject rfct::playerController::findObjectToHold()
{
	constexpr float forgivenessVine = 0.8f;
	nearestObject returnVal;
	// proritize vines
	returnVal.object = findTheNearestVineToPlayer(player);
	if (returnVal.object != flecs::entity::null()) {
		std::pair<glm::vec2, int> vineEdgePos = getNearestEdgePos((player.get<positionComponent>()->position), returnVal.object);
		returnVal.closestPosition = vineEdgePos.first;
		if (len(returnVal.closestPosition - posComp->position) < forgivenessVine) {
			returnVal.vineIndex = vineEdgePos.second;
			return returnVal;
		}
	}
	// vine is too far, fallback to block
	returnVal.object = findTheNearestBlockToPlayer(player);
	const staticBoxColliderComponent* boc = returnVal.object.get<staticBoxColliderComponent>();
	returnVal.closestPosition = nearestPointOnAABB(posComp->position, boc->min, boc->max);
	if (len(returnVal.closestPosition - posComp->position) < (forgivenessVine * 0.5f) && 
		(std::abs((returnVal.closestPosition - posComp->position).x) > std::abs((returnVal.closestPosition - posComp->position).y)))
	{
		player.get_mut<positionComponent>()->position.x = returnVal.closestPosition.x - 0.25f * ((returnVal.closestPosition.x - posComp->position.x)>0?1.f:-1.f);
		returnVal.vineIndex = -2;
		return returnVal;
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

void rfct::playerController::endHold(scene* sc)
{
	sc->getObjectHolder().nearestVineEdgeToPlayerIndex = -1;
	if (sc->getObjectHolder().vineClosestToPlayer != flecs::entity::null())
		sc->getObjectHolder().vineClosestToPlayer.get_mut<vineStateComponent>()->holdingToThis = false;
	player.get_mut<gravityComponent>()->gravityEnabled = true;

	holdCooldown = 0.4f;
	velComp->velocity.y += 2.f;
}


bool rfct::playerController::checkHold(scene* scen)
{
	if (hold && holdCooldown == 0.f) {
		nearestObjectToHold = findObjectToHold();
		if (nearestObjectToHold.vineIndex == -2) {
			stateComp->state = playerState::holdingBlocks;
		}
		else if (nearestObjectToHold.vineIndex >= 0) {
			stateComp->state = playerState::holdingVines;
			
			scen->getObjectHolder().vineClosestToPlayer = nearestObjectToHold.object;
			scen->getObjectHolder().vineClosestToPlayer.get_mut<vineStateComponent>()->holdingToThis = true;
			scen->getObjectHolder().nearestVineEdgeToPlayerIndex = nearestObjectToHold.vineIndex;
		}
		else {
			return false;
		}
		facingRight = (nearestObjectToHold.closestPosition.x - posComp->position.x) > 0;
		velComp->velocity = {0,0};
		return true;
	}
	else {
		return false;
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
	velComp->velocity = { 0,0 };
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
	/*
	if (resolution.x != 0.0f && resolution.y != 0.0f) {
		// circle collision. zero velocity on x only when real change.
		if (std::abs(resolution.x) > 0.01f) {
			vel->velocity.x = 0.0f;
		}
	}*/
	
	{
		if (resolution.x != 0.0f) {

			vel->velocity.x = 0.0f;
		}
	}
	if (resolution.y != 0.0f) {
		if (resolution.y > 0.0f) {
			// Landed on something
			vel->velocity.y = 0.0f;
			ivel->velocity.y = 0.0f;
		}
		else {
			// Hit your head on a ceiling: only stop upward motion
			if (vel->velocity.y > 0.0f) {
				vel->velocity.y = -vel->velocity.y * 0.3f;
				ivel->velocity.y = 0.0f;
			}
		}
	}
	if (resolution.y > 0)
	{
		player.get_mut<playerStateComponent>()->grounded = true;
	}
}

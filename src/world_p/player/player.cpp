#include "player.h"
#include "input.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/scene.h"
#include "world_p/physics/physics.h"
#include "world_p/transform.h"
#include "world_p/decors/dash_kindlings.h"
// sound effects
#include "sound_p/sound.h"

constexpr float maxVelocityX = 0.6f;
constexpr float walkSpeed = 6.f;
constexpr float jumpSpeed = 1.3f;
constexpr float dashSpeed = 7.f;
constexpr float boostPureHorizontalVertical = 1.2f;

rfct::playerController instance;

rfct::playerController& rfct::playerController::get() { return instance; }

// debug drawing
namespace rfct {
	void drawPlayervelocity(const glm::vec2 velComp, const glm::vec2 posComp, const glm::vec2 offset = { 0.f, 0.f }) {
		debugLine* line = debugDraw::requestLines(1);
		line->vertices[0].pos = { posComp + offset, 0 };
		line->vertices[1].pos = { posComp + (velComp)+offset, 0 };
		line->vertices[0].color = { std::clamp((float)(glm::length(velComp) / (maxVelocityX * 2)), 0.f, 1.f), 0.f, 1.f };
		line->vertices[1].color = { std::clamp((float)(glm::length(velComp) / (maxVelocityX * 2)), 0.f, 1.f), 0.f, 1.f };
	}
	float len(const glm::vec2& vec) {
		return std::sqrt((vec.x * vec.x) + (vec.y * vec.y));
	}
}

namespace rfct {
	void onCollision_Player_StaticObj(entity player, entity collidedWith, glm::vec2 resolution) {
		ecs::get().get<positionComponent>(player).position += resolution;

		velocityComponent& vel = ecs::get().get<velocityComponent>(player);
		inputVelocityComponent& ivel = ecs::get().get<inputVelocityComponent>(player);
		/*
		if (resolution.x != 0.0f && resolution.y != 0.0f) {
			// circle collision. zero velocity on x only when real change.
			if (std::abs(resolution.x) > 0.01f) {
				vel->velocity.x = 0.0f;
			}
		}*/

		{
			if (resolution.x != 0.0f) {

				vel.velocity.x = 0.0f;
			}
		}
		if (resolution.y != 0.0f) {
			if (resolution.y > 0.0f) {
				// Landed on something
				vel.velocity.y = 0.0f;
				ivel.velocity.y = 0.0f;
			}
			else {
				// Hit your head on a ceiling: only stop upward motion
				if (vel.velocity.y > 0.0f) {
					vel.velocity.y = -vel.velocity.y * 0.3f;
					ivel.velocity.y = 0.0f;
				}
			}
		}
	}
}

rfct::playerController::playerController()
	: arrowUpDownInput(0),
	walkHorizontalInput(0),
	jumpInput(0),
	dashHorizontalInput(0),
	dashVerticalInput(0),
	dash45upInput(0),
	dash45downInput(0),
	anyDash(false),
	walkVelocity(0),
	dashVelocity(0.f, 0.f),
	holdingTime(0.f),
	timeYNotZero(0),
	facingRight(true),
	nearestObjectToHold(),
	startedJumpingTime(0.f),
	dashCooldown(0.f) {
}

entity rfct::playerController::createPlayer(scene* sc, const glm::vec2& spawnPoint) {
	RFCT_PROFILE_FUNCTION();
	dynamicBoxColliderComponent bounds = { { -0.23f, -0.45f }, { 0.23, 0.4f } };

	transform trans = {};

	constexpr float oneSeventieth = 1.f / 70.f;
	trans.scale = scaleComponent{};
	trans.scale.scale.x = oneSeventieth;
	trans.scale.scale.y = oneSeventieth;
	glm::mat4 model = getModelMatrixFromTransform(trans);
	frameContext noCtx{};
	// player always uses index 1.
	sc->getRenderData().updateMat(&noCtx, 1, &model);

	staticObjCollisionCallbackComponent colCallback;
	colCallback.handler = onCollision_Player_StaticObj;
	entt::registry& reg = ecs::get();
	player = reg.create();
	reg.emplace<dynamicSSBOIndexComponent>(player, dynamicSSBOIndexComponent{ 1 });
	reg.emplace<rotationComponent>(player, rotationComponent{});
	reg.emplace<scaleComponent>(player, trans.scale);
	reg.emplace<positionComponent>(player, positionComponent{ spawnPoint });
	reg.emplace<gravityComponent>(player, gravityComponent{});
	reg.emplace<velocityComponent>(player, velocityComponent{ glm::vec2(0.f,0.f) });
	reg.emplace<inputVelocityComponent>(player, inputVelocityComponent{ glm::vec2(0.f,0.f) });
	reg.emplace<staticObjCollisionCallbackComponent>(player, colCallback);
	reg.emplace<dynamicBoxColliderComponent>(player, bounds);
	reg.emplace<playerStateComponent>(player, playerStateComponent{});
	reg.emplace<playerDashStateComponent>(player, playerDashStateComponent{});
	reg.emplace<dynamicObjectTypeComponent>(player, dynamicObjectTypeComponent{ dynamicObjectType::Player, false });
	reg.emplace<playerLifeComponent>(player, playerLifeComponent{ true });

	return player;
}

void rfct::playerController::update(frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();

	if (input::getInput().hold) {
		hold = true;
	}

	arrowUpDownInput = input::getInput().upDown;

	playerStateComponent& state = ecs::get().get<playerStateComponent>(player);
	if ((input::getInput().dashX || input::getInput().dashY || input::getInput().dash45up || input::getInput().dash45down || input::getInput().dashDefault) && state.dashCharges > 0 && dashCooldown <= 0.f) {
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

	walkHorizontalInput = 0;
	
	if (input::getInput().walk) {
		walkHorizontalInput = input::getInput().walk;
	}

	// jump
	if (input::getInput().jump && state.allowToJump) {
		jumpInput = input::getInput().jump;
	}
}

void rfct::playerController::fixedUpdate(frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	// fixed update
	playerStateComponent& state = ecs::get().get<playerStateComponent>(player);

	inputVelocityComponent& inputVelComp = ecs::get().get<inputVelocityComponent>(player);
	glm::vec2& inputVel = inputVelComp.velocity;
	inputVel = glm::vec2(0.f, 0.f);

	positionComponent& posComp = ecs::get().get<positionComponent>(player);
	velocityComponent& velComp = ecs::get().get<velocityComponent>(player);
	playerStateComponent& stateComp = ecs::get().get<playerStateComponent>(player);

	gravityComponent& grav = ecs::get().get<gravityComponent>(player);

	dashCooldown = std::clamp(dashCooldown - fixedDeltaTime, 0.f, 3.f);
	holdCooldown = std::clamp(holdCooldown - fixedDeltaTime, 0.f, 3.f);
	holdJumpCooldown = std::clamp(holdJumpCooldown - fixedDeltaTime, 0.f, .5f);

	switch (stateComp.state)
	{
	case (playerState::normal): {
		if (velComp.velocity.y == 0) {
			timeYNotZero = 0;
			stateComp.allowToJump = true;
			if (dashCooldown == 0.f) {
				stateComp.dashCharges = 1;
			}
		}
		else {
			timeYNotZero += fixedDeltaTime;
			if (timeYNotZero == fixedDeltaTime) {
				velComp.velocity.x += (facingRight ? 1.f : -1.f) * 0.6f;
			}
			if (timeYNotZero > fixedDeltaTime * 3 && stateComp.allowToJump) {
				stateComp.allowToJump = false;
			}
		}

		normalWalkUpdate();
		if (jumpInput != 0) {
			if (!(velComp.velocity.y>3.f)) {// freshly from jump booster
				startedJumpingTime = 0.f;
				stateComp.state = playerState::jumping;
			}
		}
		if (!checkHold(ctx->scene)) {
			if (anyDash) {
				dashTime = 0.f;
				stateComp.state = playerState::dashing;
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
				velComp.velocity = dashVelocity;
				inputVel = dashVelocity;
			}

			// update kindlings
			playerDashStateComponent& dc = ecs::get().get<playerDashStateComponent>(player);
			grav.gravityEnabled = false;
			dc.dashing = true;
			dc.dashProgress = dashTime / dashFullTime;

			if (dc.dashProgress > 0.1f && kindlingsToSpawnThisDash == 3) {
				kindlingsToSpawnThisDash -= 1;
				spawnKindling(ctx, posComp.position, velComp.velocity, kindlingsToSpawnThisDash);
			}
			else {
				if (dc.dashProgress > 0.3f && kindlingsToSpawnThisDash == 2) {
					kindlingsToSpawnThisDash -= 1;
					spawnKindling(ctx, posComp.position, velComp.velocity, kindlingsToSpawnThisDash);
				}
				else {
					if (dc.dashProgress > 0.9f && kindlingsToSpawnThisDash == 1) {
						kindlingsToSpawnThisDash -= 1;
						spawnKindling(ctx, posComp.position, velComp.velocity, kindlingsToSpawnThisDash);
					}
				}
			}
		}
		else {
			dashCooldown = .2f;
			// end dash
			dashTime = 0.f;
			ecs::get().get<gravityComponent>(player).gravityEnabled = true;
			stateComp.state = playerState::normal;
		}
		break;
	}
	case (playerState::jumping): {
		normalWalkUpdate();
		normalJumpUpdate();
		if (velComp.velocity.y == 0.f) {
			stateComp.allowToJump = true;
			stateComp.state = playerState::normal;
		}

		// hold priority over dash
		if (!checkHold(ctx->scene)) {
			if (anyDash) {
				dashTime = 0.f;
				stateComp.state = playerState::dashing;
			}
		}
		break;
	}
	case (playerState::holdingVines): {
		normalWalkUpdate();
		velComp.velocity *= 0.75f;
		stateComp.allowToJump = false;
		grav.gravityEnabled = false;
		if (!hold) {
			stateComp.state = playerState::normal;
		}
		if (anyDash) {
			dashTime = 0.f;
			stateComp.state = playerState::dashing;
		}
		if (stateComp.state != playerState::holdingVines) {
			endHold(ctx->scene);
		}
		break;
	}
	case (playerState::holdingBlocks): {
		holdingTime += fixedDeltaTime;
		nearestObjectToHold = findObjectToHold();
		if (((nearestObjectToHold.closestPosition.y < posComp.position.y) || (nearestObjectToHold.closestPosition.y > posComp.position.y))// out of bounds
			&& (holdingTime >= fixedDeltaTime * 4.f) // to avoid the weird bug
			) {
			stateComp.state = playerState::normal;
			posComp.position.y += std::abs(nearestObjectToHold.closestPosition.x - posComp.position.x) * 0.9f;
		}
		if (!hold) {
			stateComp.state = playerState::normal;
		}
		stateComp.allowToJump = holdJumpCooldown == 0.f;
		grav.gravityEnabled = false;
		if (jumpInput != 0 && holdJumpCooldown == 0.f) {
			startedJumpingTime = 0.f;
			stateComp.state = playerState::jumping;
		}

		float move = arrowUpDownInput;
		move = std::clamp(move, -maxVelocityX * 0.75f, maxVelocityX * 0.75f);
		velComp.velocity.y = move;
		velComp.velocity.x = 0;

		if (anyDash) {
			dashTime = 0.f;
			stateComp.state = playerState::dashing;
		}

		if (stateComp.state != playerState::holdingBlocks) {
			if (stateComp.state == playerState::normal) {
				posComp.position.x += (facingRight ? 1.f : -1.f) * 0.3f;
				posComp.position.y += 0.1f;
			}
			grav.gravityEnabled = true;
			holdCooldown = 0.25f;
			//velComp.velocity.y += .5f;
			holdJumpCooldown = 2.0f;
			holdingTime = 0.f;
		}
		break;
	}
	default:
		break;
	}

	if (velComp.velocity.x > 0) {
		facingRight = true;
	}
	else if (velComp.velocity.x < 0) {
		facingRight = false;
	}
}

void rfct::playerController::postFixedUpdate(frameContext* ctx) {
	anyDash = false;
	dashHorizontalInput = 0.f;
	dashVerticalInput = 0.f;
	dash45upInput = 0.f;
	dash45downInput = 0.f;
	arrowUpDownInput = 0.f;
	jumpInput = 0;
	hold = false;
	ctx->scene->updateDirection(facingRight);
}

rfct::nearestObject rfct::playerController::findObjectToHold() {
	RFCT_PROFILE_FUNCTION();
	constexpr float forgivenessVine = 0.8f;
	nearestObject returnVal;
	// proritize vines
	returnVal.object = findTheNearestVineToPlayer(player);
	positionComponent& posComp = ecs::get().get<positionComponent>(player);
	if (returnVal.object != entt::null) {
		std::pair<glm::vec2, int> vineEdgePos = getNearestEdgePos(posComp.position, returnVal.object);
		returnVal.closestPosition = vineEdgePos.first;
		if (len(returnVal.closestPosition - posComp.position) < forgivenessVine) {
			returnVal.vineIndex = vineEdgePos.second;
			return returnVal;
		}
	}
	// vine is too far, fallback to block
	returnVal.object = findTheNearestBlockToPlayer(player);
	
	const staticBoxColliderComponent& boc = ecs::get().get<staticBoxColliderComponent>(returnVal.object);
	returnVal.closestPosition = nearestPointOnAABB(posComp.position, boc.min, boc.max);
	if (len(returnVal.closestPosition - posComp.position) < (forgivenessVine * 0.5f) && 
		(std::abs((returnVal.closestPosition - posComp.position).x) > std::abs((returnVal.closestPosition - posComp.position).y)))
	{
		posComp.position.x = returnVal.closestPosition.x - 0.25f * ((returnVal.closestPosition.x - posComp.position.x)>0?1.f:-1.f);
		returnVal.vineIndex = -2;
		return returnVal;
	}
	// all objects too far
	returnVal.vineIndex = -1;
	returnVal.closestPosition = { 0,0 };
	returnVal.object = entity();
	return returnVal;
}

void rfct::playerController::normalWalkUpdate() {
	RFCT_PROFILE_FUNCTION();
	walkVelocity += fixedDeltaTime * walkSpeed * walkHorizontalInput;
	walkVelocity = std::clamp(walkVelocity, -maxVelocityX, maxVelocityX);


	// walk apply
	walkVelocity *= 0.85f;
	
	ecs::get().get<inputVelocityComponent>(player).velocity.x += walkVelocity;
	ecs::get().get<velocityComponent>(player).velocity.x = walkVelocity;
}

void rfct::playerController::normalJumpUpdate() {
	RFCT_PROFILE_FUNCTION();
	if (startedJumpingTime == 0.f) {
		// first 
		startedJumpingTime = 0.01f;
	}
	startedJumpingTime += fixedDeltaTime;
	float inputMultiplayer = std::clamp(- std::sqrt(7.f * startedJumpingTime) + 1.f, 0.f, 1.f);
	ecs::get().get<velocityComponent>(player).velocity.y += inputMultiplayer * jumpInput * jumpSpeed;
	ecs::get().get<inputVelocityComponent>(player).velocity.y += inputMultiplayer * jumpInput * jumpSpeed;
}

void rfct::playerController::endHold(scene* sc) {
	RFCT_PROFILE_FUNCTION();
	objectSystems::get().onEndHolding();
	ecs::get().get<gravityComponent>(player).gravityEnabled = true;

	holdCooldown = 0.4f;
	ecs::get().get<velocityComponent>(player).velocity.y += 2.f;
}

bool rfct::playerController::checkHold(scene* scen) {
	RFCT_PROFILE_FUNCTION();
	playerStateComponent& stateComp = ecs::get().get<playerStateComponent>(player);
	positionComponent& posComp = ecs::get().get<positionComponent>(player);
	velocityComponent& velComp = ecs::get().get<velocityComponent>(player);
	if (hold && holdCooldown == 0.f) {
		nearestObjectToHold = findObjectToHold();
		if (nearestObjectToHold.vineIndex == -2) {
			
			stateComp.state = playerState::holdingBlocks;
		}
		else if (nearestObjectToHold.vineIndex >= 0) {
			stateComp.state = playerState::holdingVines;
			
			objectSystems::get().onStartHolding(stateComp.state, nearestObjectToHold);
		}
		else {
			return false;
		}
		facingRight = (nearestObjectToHold.closestPosition.x - posComp.position.x) > 0;
		velComp.velocity = {0,0};
		return true;
	}
	else {
		return false;
	}
}

void rfct::playerController::startDash(frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	play(soundManager::get().swoosh);
	ecs::get().get<playerStateComponent>(player).dashCharges--;
	ecs::get().get<velocityComponent>(player).velocity = { 0.f,0.f };
	ecs::get().get<velocityComponent>(player).velocity = { 0,0 };
	dashVelocity = { 0, 0 };
	dashVelocity.y += dashVerticalInput * dashSpeed * boostPureHorizontalVertical;
	dashVelocity.x += dashHorizontalInput * dashSpeed * boostPureHorizontalVertical;
	dashVelocity.x += (dash45upInput + dash45downInput) * ((float)1 / std::sqrt(2)) * dashSpeed;
	dashVelocity.y += (dash45upInput - dash45downInput) * ((float)1 / std::sqrt(2)) * dashSpeed;

	ctx->scene->getObjectHolder().onPlayerDash(ctx, player, facingRight);
	ctx->scene->getDecorationHolder().onPlayerDashDecorations(ctx, player, facingRight);
	kindlingsToSpawnThisDash = 3;
	
	ecs::get().get<playerStateComponent>(player).allowToJump = false;
	ecs::get().get<gravityComponent>(player).gravityEnabled = false;
}
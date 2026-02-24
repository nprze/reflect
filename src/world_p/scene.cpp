#include "scene.h"

#include "ecs.h"
#include "components.h"
#include "transform.h"
#include "input.h"
#include "context.h"
#include "camera/camera.h"
#include "renderer_p/renderer.h"
#include "physics/physics.h"
#include "player/player.h"
#include "renderer_p/mesh/mesh.h"
#include "assets/object_load.h"
#include "player/player_animations.h"
#include "objects/objects.h"
#include "world.h"

rfct::scene::scene(world* worldArg) : m_World(worldArg)
{}

void rfct::scene::onUpdate(frameContext* context)
{
	RFCT_PROFILE_SCOPE("scene update");
	entt::registry& reg = ecs::get();

	debugLine* lines = debugDraw::requestLines(4);
	lines->vertices[0].pos = { 0, 0, 0 };
	lines->vertices[1].pos = { m_InitialData.width, 0, 0 };

	lines->vertices[2].pos = { m_InitialData.width, 0, 0 };
	lines->vertices[3].pos = { m_InitialData.width, m_InitialData.height, 0 };

	lines->vertices[4].pos = { m_InitialData.width, m_InitialData.height, 0 };
	lines->vertices[5].pos = { 0, m_InitialData.height, 0 };

	lines->vertices[6].pos = { 0, m_InitialData.height, 0 };lines->vertices[7].pos = { 0, 0, 0 };

	lines->vertices[0].color = { 1,1,1 };
	lines->vertices[1].color = { 1,1,1 };
	lines->vertices[2].color = { 1,1,1 };
	lines->vertices[3].color = { 1,1,1 };
	lines->vertices[4].color = { 1,1,1 };
	lines->vertices[5].color = { 1,1,1 };
	lines->vertices[6].color = { 1,1,1 };
	lines->vertices[7].color = { 1,1,1 };

	// dt update
	playerController::get().update(context);
	playerAnimations::get().update(reg.get<velocityComponent>(playerEntity).velocity, reg.get<positionComponent>(playerEntity).position, *context, playerEntity);
	updateTransformData(context, playerEntity);
	objectSystems::get().updateVisuals(context);
	m_decorations.decorsUpdate(context);
	cameraComponentOnUpdate(context->dt, playerEntity, m_InitialData.width, m_InitialData.height);
}

void rfct::scene::FixedUpdate(frameContext* context)
{
	if (!ecs::get().get<playerLifeComponent>(playerEntity).alive) {
		resetScene(context);
	}
	playerController::get().fixedUpdate(context);
	objectSystems::get().systemsFixedUpdate(context);
	m_decorations.decorsFixedUpdate(context);
	buildDynamicObjBVH();
	updatePhysics(context);
}

void rfct::scene::postFixedUpdate(frameContext* context)
{
	playerController::get().postFixedUpdate(context);
}

void rfct::scene::initScene(const std::string& path)
{
	loadScene(path, &m_InitialData);
	
	entt::registry& reg = ecs::get();


	m_World->getRenderData().startTransferStatic();
	//createStaticBackgroundMesh("background/20x20-0.txt", { 0.06f, 0.04f,0.04f });
	for (rectangle r : m_InitialData.rectangles) {
		glm::vec2 min = r.min;
		min.x -= 1;
		min.y -= 1;
		glm::vec2 size = r.max - min;
		glm::vec3 color{0.f};
		color.r = std::stoi(r.color.substr(0, 2), nullptr, 16);
		color.g = std::stoi(r.color.substr(2, 2), nullptr, 16);
		color.b = std::stoi(r.color.substr(4, 2), nullptr, 16);
		createStaticMesh("building_blocks/" + r.file, size, r.min, color);
	}

	playerEntity = playerController::get().createPlayer(this, m_InitialData.spawnPoints[0].position);

	camera = reg.create();
	reg.emplace<position3DComponent>(camera, position3DComponent{ { m_InitialData.spawnPoints[0].position, 20.f} });
	reg.emplace<rotationComponent>(camera, rotationComponent{ {0.f, 0.f, 0.f} });
	reg.emplace<cameraComponent>(camera, cameraComponent{ CameraFOV, renderer::getRen().getAspectRatio(), 0.1f, 100.0f });
	setCamera(camera);

	// init dynamic objects
	objectSystems::get().loadSceneData(&m_InitialData, this);
	m_decorations.init(&m_InitialData, this);


	//if (m_InitialData.vines.size() != 0) hasVines = true;

	// init player hair anim
	;
	const dynamicBoxColliderComponent& bounds = reg.get<dynamicBoxColliderComponent>(playerEntity);
	playerAnimations::get().initHairAnim(bounds.max.x - bounds.min.x, bounds.max.y - bounds.min.y);

	m_World->getRenderData().endTransferStatic();

	buildStaticObjBVH();
	buildDynamicObjBVH();

	m_pendingEntityDeletions.clear();
	m_pendingEntityDeletions.reserve(20);

	m_InitialData.rectangles.clear();
	
	//getRenderData().clearAllData();
}

void rfct::scene::unloadScene()
{
}

rfct::renderData& rfct::scene::getRenderData()
{
	return m_World->getRenderData();
}



entity rfct::scene::createStaticMesh(const std::string& path, glm::vec2 size, glm::vec2 pos, const glm::vec3& color)
{
	buildingBlockMesh mesh1(path, color, size * 70.f);

	staticBoxColliderComponent collider;
	collider.min = pos;
	collider.max.x = pos.x + size.x;
	collider.max.y = pos.y + size.y;
	transform transform1;
	transform1.scale.scale.x = 1.f / 70.f;
	transform1.scale.scale.y = 1.f / 70.f;

	transform1.pos.position = pos;
	glm::mat4 model = getModelMatrixFromTransform(transform1);
	objectLocation ol = m_World->getRenderData().addStaticObject(&mesh1.m_Vertices, &model);
	staticSSBOIndexComponent ssboIndex = { ol.indexInSSBO };

	entt::registry& reg = ecs::get();
	entity e = reg.create();
	reg.emplace<staticSSBOIndexComponent>(e, staticSSBOIndexComponent{ ol.indexInSSBO });
	reg.emplace<vertexRenderInfoComponent>(e, vertexRenderInfoComponent{ ol.verticesCount, ol.vertexBufferOffset });
	reg.emplace<positionComponent>(e, positionComponent{});
	reg.emplace<rotationComponent>(e, rotationComponent{});
	reg.emplace<scaleComponent>(e, transform1.scale);
	reg.emplace<staticBoxColliderComponent>(e, collider);
	return e;
}

entity rfct::scene::createStaticBackgroundMesh(const std::string& path, const glm::vec3& color, const float zMin, const float zMax)
{
	backgroundMesh mesh1(path, color, zMin, zMax);


	glm::mat4 model = glm::mat4(1.f);
	objectLocation ol = m_World->getRenderData().addStaticObject(&mesh1.m_Vertices, &model);
	staticSSBOIndexComponent ssboIndex = { ol.indexInSSBO };
	entt::registry& reg = ecs::get();
	entity e = reg.create();
	reg.emplace<staticSSBOIndexComponent>(e, staticSSBOIndexComponent{ ol.indexInSSBO });
	reg.emplace<vertexRenderInfoComponent>(e, vertexRenderInfoComponent{ ol.verticesCount, ol.vertexBufferOffset });
	reg.emplace<positionComponent>(e, positionComponent{});
	return e;
}

entity rfct::scene::createStaticRect(staticBoxColliderComponent* bounds, glm::vec3 color)
{
	std::vector<Vertex> vertices = {
		{{bounds->min.x, bounds->min.y, 0.f}, color,0,0},
		{{bounds->min.x, bounds->max.y, 0.f}, color,0,0},
		{{bounds->max.x, bounds->max.y, 0.f}, color,0,0},
		{{bounds->max.x, bounds->max.y, 0.f}, color,0,0},
		{{bounds->min.x, bounds->min.y, 0.f}, color,0,0},
		{{bounds->max.x, bounds->min.y, 0.f}, color,0,0},
	};
	transform trans = {};
	glm::mat4 model = getModelMatrixFromTransform(trans);
	entity e = createStaticRenderingEntity(&vertices, &model);
	ecs::get().emplace<staticBoxColliderComponent>(e, *bounds);
	return e;
}

entity rfct::scene::createDynamicRect(dynamicBoxColliderComponent* bounds, glm::vec3 color)
{
	std::vector<Vertex> vertices = {
		{{bounds->min.x, bounds->min.y, 0.f},	color,0,0},
		{{bounds->min.x, bounds->max.y, 0.f}, color,0,0},
		{{bounds->max.x, bounds->max.y, 0.f}, color,0,0},
		{{bounds->max.x, bounds->max.y, 0.f}, color,0,0},
		{{bounds->min.x, bounds->min.y, 0.f}, color,0,0},
		{{bounds->max.x, bounds->min.y, 0.f}, color,0,0},
	};
	transform trans = {};
	glm::mat4 model = getModelMatrixFromTransform(trans);
	entity e = createDynamicRenderingEntity(&vertices, &model);
	ecs::get().emplace<dynamicBoxColliderComponent>(e, *bounds);
	return e;
}

entity rfct::scene::createDynamicMesh(dynamicBoxColliderComponent* bounds, const std::string& path)
{
	mesh mesh1(path);

	transform trans = {};

	glm::mat4 model = getModelMatrixFromTransform(trans);
	entity e = createDynamicRenderingEntity(&mesh1.m_Vertices, &model);
	ecs::get().emplace<dynamicBoxColliderComponent>(e, *bounds);
	return e;
}

entity rfct::scene::createStaticRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model)
{
	objectLocation ol = m_World->getRenderData().addStaticObject(vertices, model);
	staticSSBOIndexComponent ssboIndex = { ol.indexInSSBO };


	entt::registry& reg = ecs::get();
	entity e = reg.create();
	reg.emplace<staticSSBOIndexComponent>(e, staticSSBOIndexComponent{ ol.indexInSSBO });
	reg.emplace<vertexRenderInfoComponent>(e, vertexRenderInfoComponent{ ol.verticesCount, ol.vertexBufferOffset });
	reg.emplace<positionComponent>(e, positionComponent{});
	reg.emplace<rotationComponent>(e, rotationComponent{});
	reg.emplace<scaleComponent>(e, scaleComponent{});
	return e;
}

void rfct::scene::deleteDynamicEntity(entity e)
{
	m_World->getRenderData().removeDynamicEntity(e);
	ecs::get().destroy(e);
}

void rfct::scene::deleteAnimatedEntity(entity e)
{
	m_World->getRenderData().removeAnimatedEntity(e);
	ecs::get().destroy(e);
}

entity rfct::scene::createDynamicRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model, uint32_t numVertices)
{
	objectLocation ol = m_World->getRenderData().addDynamicObject(vertices, model, {}, numVertices);

	entt::registry& reg = ecs::get();
	entity e = reg.create();
	reg.emplace<dynamicSSBOIndexComponent>(e, dynamicSSBOIndexComponent{ ol.indexInSSBO });
	reg.emplace<vertexRenderInfoComponent>(e, vertexRenderInfoComponent{ ol.verticesCount, ol.vertexBufferOffset });
	reg.emplace<positionComponent>(e, positionComponent{});
	reg.emplace<rotationComponent>(e, rotationComponent{});
	return e;
}

void rfct::scene::updateTransformData(const frameContext* ctx, entity e)
{
	glm::mat4 model = getModelMatrixFromEntity(e);
	m_World->getRenderData().updateMat(ctx, ecs::get().get<dynamicSSBOIndexComponent>(e).indexInSSBO, &model);
}

void rfct::scene::updateDirection(bool facingRight)
{
	
	scaleComponent& scale = ecs::get().get<scaleComponent>(playerEntity);
	scale.scale.x = scale.scale.x* (facingRight? (scale.scale.x < 0 ? -1 : 1) :(scale.scale.x>0?-1:1));
}

bool rfct::scene::isPlayerOutsideScene()
{
	
	glm::vec2 pos = ecs::get().get<positionComponent>(playerEntity).position;
	if (pos.x > m_InitialData.width || pos.x < 0) return true;
	if (pos.y > m_InitialData.height || pos.y < 0) return true;
	return false;
}

glm::vec2 rfct::scene::getPlayerCoordsSceneNormalized()
{
	glm::vec2 pos = ecs::get().get<positionComponent>(playerEntity).position;
	pos.x /= m_InitialData.width;
	pos.y /= m_InitialData.height;
	return {std::clamp(pos.x, 0.f, 1.f), std::clamp(pos.y, 0.f, 1.f) };
}

void rfct::scene::resetScene(frameContext* ctx)
{
	ecs::get().get<playerLifeComponent>(playerEntity).alive = true;
	playerController::get().endHold(this);
	ecs::get().get<positionComponent>(playerEntity).position = m_InitialData.spawnPoints[0].position;
	ecs::get().get<velocityComponent>(playerEntity).velocity = {0,0};
	ecs::get().get<playerStateComponent>(playerEntity).state = playerState::normal;
	objectSystems::get().respawn(ctx);
}

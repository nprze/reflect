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
#include "assets/assets_manager.h"
#include "ui.h"
#include <stb_image/stb_image.h>
#include "player/player_animations.h"
#include "assets/dialogue_serialize_data.h"
#include "objects/objects.h"

const float maxVelocityX = 100;
rfct::scene::scene(world* worldArg) : m_World(worldArg)
{
	
}

rfct::scene::~scene()
{
	cleanupQueries();
}
namespace rfct{
	void drawGridLines(int n, int start_from = 0, const glm::vec3 colorPositive = { 0.6f,0.6f,0.6f }, const glm::vec3 colorNegative = { 0.4f,0.4f,0.4f }) {
		const float z_coord = 0.0f;

		// We need (n + 1) vertical + (n + 1) horizontal lines
		int totalLines = 2 * (n + 1);
		debugLine* lines = debugDraw::requestLines(totalLines);

		int lineIndex = 0;

		// Draw vertical lines (parallel to Y-axis)
		for (int i = 0; i <= n; ++i) {
			float x = start_from + i;
			glm::vec3 color = (x < 0) ? colorNegative : colorPositive;

			lines[lineIndex].vertices[0].pos = { x, start_from, z_coord };
			lines[lineIndex].vertices[1].pos = { x,  start_from+n, z_coord };
			lines[lineIndex].vertices[0].color = color;
			lines[lineIndex].vertices[1].color = color;

			++lineIndex;
		}

		// Draw horizontal lines (parallel to X-axis)
		for (int i = 0; i <= n; ++i) {
			float y = start_from + i;
			glm::vec3 color = (y < 0) ? colorNegative : colorPositive;

			lines[lineIndex].vertices[0].pos = { start_from, y, z_coord };
			lines[lineIndex].vertices[1].pos = { start_from+n, y, z_coord };
			lines[lineIndex].vertices[0].color = color;
			lines[lineIndex].vertices[1].color = color;

			++lineIndex;
		}
	}
}

void rfct::scene::onUpdate(frameContext* context)
{
	RFCT_PROFILE_SCOPE("scene update");
	if (!epicRotatingTriangle.get<playerLifeComponent>()->alive) {
		resetScene(context);
	}
	playerController::get().update(context);
	objectsHolder::get().update(context);
	m_decorations.update(context);
	buildDynamicObjBVH();
	updatePhysics(context);
	updateUI(context);
	playerAnimations::get().update(epicRotatingTriangle.get<velocityComponent>()->velocity, epicRotatingTriangle.get<positionComponent>()->position, *context, epicRotatingTriangle);
	updateTransformData(context, epicRotatingTriangle);

	cameraComponentOnUpdate(context->dt, epicRotatingTriangle);

	objectsHolder::get().updateVisuals(context);
	resolvePendingDynamicEnitityDeletions();
}

void rfct::scene::updateUI(frameContext* context)
{
	RFCT_PROFILE_SCOPE("UI update");
	UpdateUI(context);
}

void rfct::scene::loadScene(const std::string& path)
{
	AssetsManager::get().loadScene(path, &m_InitialData);


	sceneEntity = ecs::get().entity<sceneComponent>();
	createQueries(sceneEntity);

	camera = ecs::get().entity()
		.child_of(sceneEntity)
		.set<position3DComponent>({ { 0.f,  0.f, 20.f} })
		.set<rotationComponent>({ {0.f, 0.f, 0.f} })
		.set<cameraComponent>({ 45.0f, renderer::getRen().getAspectRatio(), 0.1f, 100.0f });
	setCamera(camera);
	m_RenderData.startTransferStatic();
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
	createPlayerEntity(m_InitialData.spawnPoints[0].position);


	// init dynamic objects
	objectsHolder::get().loadSceneData(&m_InitialData, this);
	m_decorations.init(&m_InitialData, this);


	//if (m_InitialData.vines.size() != 0) hasVines = true;

	// init player hair anim
	const dynamicBoxColliderComponent* bounds = epicRotatingTriangle.get<dynamicBoxColliderComponent>();
	playerAnimations::get().initHairAnim(bounds->max.x - bounds->min.x, bounds->max.y - bounds->min.y);

	m_RenderData.endTransferStatic();
	buildStaticObjBVH();
	buildDynamicObjBVH();

	m_pendingEntityDeletions.clear();
	m_pendingEntityDeletions.reserve(20);

	m_InitialData.rectangles.clear();
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
	objectLocation ol = m_RenderData.addStaticObject(&mesh1.m_Vertices, &model);
	staticSSBOIndexComponent ssboIndex = { ol.indexInSSBO };
	return ecs::get().entity<>()
		.child_of(sceneEntity)
		.set<staticSSBOIndexComponent>({ ol.indexInSSBO })
		.set<vertexRenderInfoComponent>({ ol.verticesCount, ol.vertexBufferOffset })
		.set<positionComponent>({})
		.set<rotationComponent>({})
		.set<scaleComponent>(transform1.scale)
		.set<staticBoxColliderComponent>(collider);
}

entity rfct::scene::createStaticBackgroundMesh(const std::string& path, const glm::vec3& color, const float zMin, const float zMax)
{
	backgroundMesh mesh1(path, color, zMin, zMax);


	glm::mat4 model = glm::mat4(1.f);
	objectLocation ol = m_RenderData.addStaticObject(&mesh1.m_Vertices, &model);
	staticSSBOIndexComponent ssboIndex = { ol.indexInSSBO };
	return ecs::get().entity<>()
		.child_of(sceneEntity)
		.set<staticSSBOIndexComponent>({ ol.indexInSSBO })
		.set<vertexRenderInfoComponent>({ ol.verticesCount, ol.vertexBufferOffset })
		.set<positionComponent>({});
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
	return createStaticRenderingEntity(&vertices, &model).set<staticBoxColliderComponent>(*bounds);
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
	return createDynamicRenderingEntity(&vertices, &model).set<dynamicBoxColliderComponent>(*bounds);
}

entity rfct::scene::createDynamicMesh(dynamicBoxColliderComponent* bounds, const std::string& path)
{
	mesh mesh1(path);

	transform trans = {};

	glm::mat4 model = getModelMatrixFromTransform(trans);
	return createDynamicRenderingEntity(&mesh1.m_Vertices, &model).set<dynamicBoxColliderComponent>(*bounds);
}

entity rfct::scene::createStaticRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model)
{
	objectLocation ol = m_RenderData.addStaticObject(vertices, model); 
	staticSSBOIndexComponent ssboIndex = { ol.indexInSSBO };
	return ecs::get().entity<>()
		.child_of(sceneEntity)
		.set<staticSSBOIndexComponent>({ ol.indexInSSBO })
		.set<vertexRenderInfoComponent>({ ol.verticesCount, ol.vertexBufferOffset })
		.set<positionComponent>({})
		.set<rotationComponent>({})
		.set<scaleComponent>({});
}

void rfct::scene::deleteDynamicEntity(entity e)
{
	m_RenderData.removeDynamicEntity(e);
	e.destruct();
}

void rfct::scene::deleteAnimatedEntity(entity e)
{
	m_RenderData.removeAnimatedEntity(e);
	e.destruct();
}

void rfct::scene::addPendingDynamicEnitityDeletion(entity e)
{
	m_pendingEntityDeletions.push_back({e, false});
}

void rfct::scene::addPendingAnimatedEnitityDeletion(entity e)
{
	m_pendingEntityDeletions.push_back({ e, true });
}

void rfct::scene::resolvePendingDynamicEnitityDeletions()
{
	for (auto& e : m_pendingEntityDeletions) {
		if (e.second)
			deleteAnimatedEntity(e.first);
		else
			deleteDynamicEntity(e.first);
	}
	m_pendingEntityDeletions.clear();
}

entity rfct::scene::createDynamicRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model, uint32_t numVertices)
{
	objectLocation ol = m_RenderData.addDynamicObject(vertices, model, true, {}, numVertices);
	dynamicSSBOIndexComponent ssboIndex = { ol.indexInSSBO };

	return ecs::get().entity<>()
		.child_of(sceneEntity)
		.set<dynamicSSBOIndexComponent>({ ol.indexInSSBO })
		.set<vertexRenderInfoComponent>({ ol.verticesCount, ol.vertexBufferOffset })
		.set<positionComponent>({})
		.set<rotationComponent>({});
}

void rfct::scene::updateTransformData(const frameContext* ctx, entity e)
{
	glm::mat4 model = getModelMatrixFromEntity(e);
	m_RenderData.updateMat(ctx, e.get<dynamicSSBOIndexComponent>()->indexInSSBO, &model);
}

void rfct::scene::createPlayerEntity(const glm::vec2& spawnPoint)
{
	epicRotatingTriangle = playerController::get().createPlayer(this, spawnPoint);
}

void rfct::scene::updateDirection(bool facingRight)
{
	scaleComponent* scale = epicRotatingTriangle.get_mut<scaleComponent>();
	scale->scale.x = scale->scale.x* (facingRight? (scale->scale.x < 0 ? -1 : 1) :(scale->scale.x>0?-1:1));
	epicRotatingTriangle.set<scaleComponent>(*scale);
}

void rfct::scene::resetScene(frameContext* ctx)
{
	epicRotatingTriangle.get_mut<playerLifeComponent>()->alive = true;
	playerController::get().endHold(this);
	epicRotatingTriangle.get_mut<positionComponent>()->position = m_InitialData.spawnPoints[0].position;
	epicRotatingTriangle.get_mut<velocityComponent>()->velocity = {0,0};
	epicRotatingTriangle.get_mut<playerStateComponent>()->state = playerState::normal;
	objectsHolder::get().reset(ctx);
}

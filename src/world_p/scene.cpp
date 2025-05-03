#include "scene.h"

#include "ecs.h"
#include "components.h"
#include "transform.h"
#include "input.h"
#include "context.h"
#include "camera/camera.h"
#include "renderer_p/renderer.h"
#include "physics/physics.h"
#include "physics/collision_callback.h"
#include "renderer_p/mesh/mesh.h"
#include "assets/assets_manager.h"

rfct::scene::scene(world* worldArg) : m_World(worldArg), m_Image0("dialogues/cat.png"), m_Image1("dialogues/cat1.png")
{
	
}

rfct::scene::~scene()
{
	cleanupQueries();
}

namespace rfct {
	void updateGamplay(float dt, entity player) {
		playerStateComponent* playerState = player.get_mut<playerStateComponent>();
		positionComponent* pos = player.get_mut<positionComponent>();
		if (input::getInput().xAxis) {
			pos->position.x += 3 * input::getInput().xAxis * dt;
		}
		if (input::getInput().yAxis && playerState->grounded) {
			playerState->grounded = false;
			player.get_mut<velocityComponent>()->velocity.y += input::getInput().yAxis * 175.f;
		}
	}
}


void rfct::scene::onUpdate(frameContext* context)
{
	updateGamplay(context->dt, epicRotatingTriangle);
	updatePhysics(context->dt);
	updateTransformData(context, epicRotatingTriangle);
	updateUI(context->dt);
	cameraComponentOnUpdate(context->dt, epicRotatingTriangle);
}

void rfct::scene::updateUI(float dt)
{
	renderer::getRen().getUIPipeline().addImage({ 0.f,20.f }, { 200.f, 100.f }, &m_Image0);
	renderer::getRen().getUIPipeline().addImage({ 0.f, 120.f }, { 200.f, 220.f }, &m_Image1);
}


void rfct::scene::loadScene(const std::string& path)
{
	sceneEntity = ecs::get().entity<sceneComponent>();
	createQueries(sceneEntity);

	camera = ecs::get().entity()
		.child_of(sceneEntity)
		.set<position3DComponent>({ { 0.f,  0.f, 20.f} })
		.set<rotationComponent>({ {0.f, 0.f} })
		.set<cameraComponent>({ 45.0f, renderer::getRen().getAspectRatio(), 0.1f, 100.0f });
	setCamera(camera);
	m_RenderData.startTransferStatic();
	{
		staticBoxColliderComponent bounds = { { 8.f, -2.f }, { 9.f, -1.f } };
		createStaticRect(&bounds, glm::vec3(0.2f, 0.7f, 0.9f));
	}
	{
		staticBoxColliderComponent bounds = { { -5.f, -3.f }, { -2.f, -2.5f } };
		createStaticRect(&bounds, glm::vec3(0.7f, 0.2f, 0.9f));

	}
	{
		staticBoxColliderComponent bounds = { { -7.f, 0.f }, { 7.f, 1.f } };
		createStaticRect(&bounds);
	}
	{
		createStaticMesh("building_blocks/700x70.txt", glm::vec2(10.f, 1.f), glm::vec2(8.f, 2.f));
	}
	{
		createStaticMesh("building_blocks/700x70.txt", glm::vec2(10.f, 1.f), glm::vec2(-8.f, 5.f));
	}
	{
		dynamicBoxColliderComponent bounds = { { -0.5f, -0.5f }, { 0.5f, 0.5f } };
		epicRotatingTriangle = createDynamicRect(&bounds, glm::vec3(0.2f, 0.7f, 0.4f));
		collisionCallbackComponent colCallback;
		colCallback.handler = onCollision_Player_StaticObj;
		epicRotatingTriangle.set<positionComponent>({ { 0.f, 6.f } }).set<gravityComponent>({}).set<velocityComponent>({ glm::vec3(0.f,0.f,0.f) }).set<collisionCallbackComponent>(colCallback).set<playerStateComponent>({});

	}
	m_RenderData.endTransferStatic();
	buildBVH();
}

entity rfct::scene::createStaticMesh(const std::string& path, glm::vec2 size, glm::vec2 pos)
{
	mesh mesh1(path);
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

entity rfct::scene::createDynamicRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model)
{
	objectLocation ol = m_RenderData.addDynamicObject(vertices, model);
	dynamicSSBOIndexComponent ssboIndex = { ol.indexInSSBO };
	return ecs::get().entity<>()
		.child_of(sceneEntity)
		.set<dynamicSSBOIndexComponent>({ ol.indexInSSBO })
		.set<vertexRenderInfoComponent>({ ol.verticesCount, ol.vertexBufferOffset })
		.set<positionComponent>({})
		.set<rotationComponent>({})
		.set<scaleComponent>({});
}

void rfct::scene::updateTransformData(frameContext* ctx, entity e)
{
	glm::mat4 model = getModelMatrixFromEntity(e);
	m_RenderData.updateMat(ctx, e.get<dynamicSSBOIndexComponent>()->indexInSSBO, &model);
}
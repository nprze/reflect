#include "scene.h"
#include "ecs.h"
#include "components.h"
#include "transform.h"
#include "input.h"
#include "context.h"
#include "camera\camera.h"
#include "renderer_p\renderer.h"
#include "physics\physics.h"

rfct::scene::scene(world* worldArg) : m_World(worldArg)
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
		if (input::getInput().yAxis) {
			player.get_mut<velocityComponent>()->velocity.y -= input::getInput().yAxis * 20 * dt;
		}
	}
}


void rfct::scene::onUpdate(frameContext* context)
{
	updateGamplay(context->dt, epicRotatingTriangle);
	updateTransformData(context, epicRotatingTriangle);
	updatePhysics(context->dt, sceneEntity);
	cameraComponentOnUpdate(context->dt, epicRotatingTriangle);

}


void rfct::scene::loadScene(std::string path)
{
	sceneEntity = ecs::get().entity<sceneComponent>();
	createQueries(sceneEntity);
	camera = ecs::get().entity()
		.child_of(sceneEntity)
		.set<position3DComponent>({ { 0.f,  0.f, 8.f} })
		.set<rotationComponent>({ {0.f, 0.f} })
		.set<cameraComponent>({ 45.0f, renderer::getRen().getAspectRatio(), 0.1f, 100.0f });
	setCamera(camera);
	m_RenderData.startTransferStatic();
	{
		staticBoxColliderComponent bounds = { { 3.f, -3.f }, { 7.f, -2.f } };
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
		dynamicBoxColliderComponent bounds = { { -0.5f, -0.5f }, { 0.5f, 0.5f } };
		epicRotatingTriangle = createDynamicRect(&bounds, glm::vec3(0.2f, 0.7f, 0.4f));
		epicRotatingTriangle.set<positionComponent>({ { 0.f, -6.f } }).set<gravityComponent>({}).set<velocityComponent>({ glm::vec3(0.f,0.f,0.f)});

	}
	m_RenderData.endTransferStatic();
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
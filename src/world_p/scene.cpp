#include "scene.h"
#include "ecs.h"
#include "components.h"
#include "transform.h"
#include "input.h"
#include "context.h"
#include "camera\camera.h"
#include "renderer_p\renderer.h"

rfct::scene::scene(world* worldArg) : m_World(worldArg)
{
}

rfct::scene::~scene()
{
}


void rfct::scene::onUpdate(frameContext* context)
{
	positionComponent* pos = epicRotatingTriangle.get_mut<positionComponent>();
	if (input::getInput().xAxis) {
		pos->position.x += 3 * input::getInput().xAxis * context->dt;
	}
	if (input::getInput().yAxis) {
		pos->position.y += 3 * input::getInput().yAxis * context->dt;
	}
	updateTransformData(context, epicRotatingTriangle);
	cameraComponentOnUpdate(context->dt, epicRotatingTriangle);

}


void rfct::scene::loadScene(std::string path)
{
	sceneEntity = ecs::get().entity<sceneComponent>();
	camera = ecs::get().entity()
		.child_of(sceneEntity)
		.set<positionComponent>({ { 0.f,  0.f, 8.f} })
		.set<rotationComponent>({ {0.f, 0.f, 0.f} })
		.set<cameraComponent>({ 45.0f, renderer::getRen().getAspectRatio(), 0.1f, 100.0f });
	setCamera(camera);
	m_RenderData.startTransferStatic();
	{

		std::vector<Vertex> vertices = {
			{{0.0f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f},0,0},
			{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f},0,0},
			{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f},0,0}
		};
		transform trans = {};
		trans.pos.position.x += 1.f;
		glm::mat4 model = getModelMatrixFromTransform(trans);
		createStaticRenderingEntity(&vertices, &model);
	}
	{

		std::vector<Vertex> vertices = {
			{{0.0f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f},0,0},
			{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f},0,0},
			{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f},0,0}
		};
		transform trans = {};
		trans.pos.position.x -= 1.f;
		glm::mat4 model = getModelMatrixFromTransform(trans);
		createStaticRenderingEntity(&vertices, &model);

	}
	{

		std::vector<Vertex> vertices = {
			{{0.0f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f},0,0},
			{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f},0,0},
			{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f},0,0}
		};
		transform trans = {};
		glm::mat4 model = getModelMatrixFromTransform(trans);
		epicRotatingTriangle = createDynamicRenderingEntity(&vertices, &model);

	}
	m_RenderData.endTransferStatic();
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
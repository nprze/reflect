#pragma once
#include "render_data.h"
#include "renderer_p/image/bindable_image.h"
#include "player/player.h"
#include "objects/objects.h"
#include "decors/decorations.h"
#include "assets/serialize_structures/scene_serialize_data.h"

namespace rfct {
	class world;
	class scene {
	public:
		scene(world* worldArg);
		void onUpdate(frameContext* context);
		void FixedUpdate(frameContext* context);
		void postFixedUpdate(frameContext* context);
		void initScene(const std::string& path);
		renderData& getRenderData();
		// all static entities can only be created during loadScene() and their render data should not change (that includes position, color, size etc.)
		entity createStaticMesh(const std::string& path, glm::vec2 size, glm::vec2 pos, const glm::vec3& color); // loads mesh from .txt file (path should be pointing to a .txt). pos is left top coord.
		entity createStaticBackgroundMesh(const std::string& path, const glm::vec3& color, const float zMin = -1, const float zMax = -20); // loads mesh from .txt file (path should be pointing to a .txt). pos is left top coord.
		entity createStaticRect(staticBoxColliderComponent* bounds, glm::vec3 color = glm::vec3(1.f, 1.f, 1.f)); // creates a simple rect with color
		entity createStaticRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model);
		void deleteDynamicEntity(entity e);
		void deleteAnimatedEntity(entity e);
		entity createDynamicRect(dynamicBoxColliderComponent* bounds, glm::vec3 color = glm::vec3(1.f, 1.f, 1.f));
		entity createDynamicMesh(dynamicBoxColliderComponent* bounds, const std::string& path);
		entity createDynamicRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model, uint32_t numVertices = 0);
		void updateTransformData(const frameContext* ctx, entity entityToUpdate); // entity must contain positionComponent, rotationComponent and scaleComponent
		void updateDirection(bool facingRight);
		bool isPlayerOutsideScene();
		glm::vec2 getPlayerCoordsSceneNormalized(); // get the coordinates of where the current player is. at the left top edge (0,0) at the bottom right (1,1)
		void resetScene(frameContext* ctx);

		entity getPlayer() { return playerEntity; }
		objectSystems& getObjectHolder() { return objectSystems::get(); }
		decorationHolder& getDecorationHolder() { return m_decorations; }
		world* getWorldScene() { return m_World; }
	public:
		entity camera;
	private: 
		world* m_World;
		decorationHolder m_decorations;
		entity playerEntity;
		std::vector<std::pair<entity, bool>> m_pendingEntityDeletions;
		sceneSerializedData m_InitialData; // holds an empty static rectangles data
	};
};
#pragma once
#include "render_data.h"
#include "renderer_p/image/bindable_image.h"
#include "player/player.h"
#include "objects/objects.h"

namespace rfct {
	class world;
	struct frameContext;
	class scene {
	public:
		scene(world* worldArg);
		~scene();

		void onUpdate(frameContext* context);
		void updateUI(frameContext* context);
		void loadScene(const std::string& path);
		inline void unloadScene() {};
		inline sceneRenderData& getRenderData() { return m_RenderData; };

		// all static entities can only be created during loadScene() and their render data should not change (that includes position, color, size etc.)
		entity createStaticMesh(const std::string& path, glm::vec2 size, glm::vec2 pos, const glm::vec3& color); // loads mesh from .txt file (path should be pointing to a .txt). pos is left top coord.
		entity createStaticRect(staticBoxColliderComponent* bounds, glm::vec3 color = glm::vec3(1.f, 1.f, 1.f)); // creates a simple rect with color
		entity createStaticRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model);
		
		entity createDynamicRect(dynamicBoxColliderComponent* bounds, glm::vec3 color = glm::vec3(1.f, 1.f, 1.f));
		entity createDynamicMesh(dynamicBoxColliderComponent* bounds, const std::string& path);
		entity createDynamicRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model);
		void updateTransformData(frameContext* ctx, entity entityToUpdate); // entity must contain positionComponent, rotationComponent and scaleComponent
		void createPlayerEntity();
		void updateDirection(bool facingRight);


		objectsHolder& getObjectHolder() { return m_dynamicObjects; }
		world* getWorld() { return m_World; }

		entity camera;
		entity sceneEntity; // root of all objects in this scene. 
		sceneRenderData m_RenderData;
	private:
		bool hasVines = false; 
		world* m_World;

		objectsHolder m_dynamicObjects;

		entity epicRotatingTriangle;
		playerController m_playerController;
	};
};
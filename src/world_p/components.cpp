#include "components.h"
#include "scene.h"
#include "transform.h"
#include "input.h"
#include "flecs\flecs.h"
#include "ecs.h"
namespace rfct {
	void registerComponents()
	{
		ecs::get().component<sceneComponent>();
		ecs::get().component<cameraComponent>();
		ecs::get().component<positionComponent>();
		ecs::get().component<position3DComponent>();
		ecs::get().component<rotationComponent>();
		ecs::get().component<scaleComponent>();
		ecs::get().component<matrixComponent>();
		ecs::get().component<staticSSBOIndexComponent>();
		ecs::get().component<dynamicSSBOIndexComponent>();
		ecs::get().component<vertexRenderInfoComponent>();
		ecs::get().component<staticBoxColliderComponent>();
		ecs::get().component<dynamicBoxColliderComponent>();
		ecs::get().component<gravityComponent>();
		ecs::get().component<velocityComponent>();
		ecs::get().component<playerStateComponent>();

		

	}


}
#pragma once
#include <string>
#include "assets/serialize_structures/world_serialize_data.h"

namespace rfct {
	class scene;
	class renderData;
	class world {
	public:
		static world& getWorld();
	public:
		void initWorld(const std::string& path);
		void loadScene(const std::string& path);
		void cleanWorld();
		void worldFixedUpdate(RfctFrameContext& context, uint64_t timesToUpdate);
		void worldVisualUpdate(RfctFrameContext& context);
        void addScreenTransform(float degree);
		void startSwitchScene(RfctFrameContext& ctx);
		void switchScenes(RfctFrameContext& ctx);
		uint32_t getSceneToLoad(glm::vec2& lastBlockExit);
		renderData& getRenderData() { return *m_RenderData; };
		scene& getCurrentScene() { return *m_currentScene; };
	public:
		bool switchingScenes = false;
		float changeSceneEffectMultiplier = 1.f;
        float screenViewTransformDegrees = 0;
	private:
		scene* m_currentScene;
		renderData* m_RenderData = nullptr;
		worldSerializeData m_serializeData;
		uint32_t m_currentWorldBlockIndex = 1;
		glm::vec2 worldCoords;
	};
}
#pragma once

namespace rfct {
	struct sceneSerializedData;
	class scene;
	void spawnTallGrass(scene* parentScene, sceneSerializedData* sd);
	void updateGrass(RfctFrameContext* ctx);
}
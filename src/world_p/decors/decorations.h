#pragma once
#include "context.h"
namespace rfct {
	struct sceneSerializedData;
	struct decorationHolder {
		void init(sceneSerializedData* serializeData, scene* parentScene);
		void onPlayerDashDecorations(frameContext* fc, const entity entityPlayer, const bool facingRight);
		void update(frameContext* ctx);
	};
}
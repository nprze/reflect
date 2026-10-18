#pragma once

namespace rfct {
	struct sceneSerializedData;
	struct decorationHolder {
		void init(sceneSerializedData* serializeData, scene* parentScene);
		void onPlayerDashDecorations(RfctFrameContext* fc, const entity entityPlayer, const bool facingRight);
		void decorsFixedUpdate(RfctFrameContext* ctx);
		void decorsUpdate(RfctFrameContext* ctx);
	};
}
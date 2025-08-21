#include "decorations.h"
#include "renderer_p/debug/debug_draw.h"
#include "smokes.h"
#include "world_p/components.h"
#include "assets/scene_serialize_data.h"

void rfct::decorationHolder::init(sceneSerializedData* serializeData, scene* parentScene) {
	initSmokeVars(parentScene);
}

rfct::decorationHolder::~decorationHolder()
{
	cleanupSmokes();
}

void rfct::decorationHolder::onPlayerDashDecorations(frameContext* fc, const entity entityPlayer, const bool facingRight) {
}
void rfct::decorationHolder::update(frameContext* ctx) {
	RFCT_PROFILE_SCOPE("decorations update");
	updateSmokes(ctx);
	updateSmokeMatrices(ctx);
}
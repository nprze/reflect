#include "decorations.h"
#include "renderer_p/debug/debug_draw.h"
#include "smokes.h"
#include "world_p/components.h"
#include "assets/scene_serialize_data.h"

void rfct::decorationHolder::init(sceneSerializedData* serializeData, scene* parentScene) {
	initSmokeVars();
}

void rfct::decorationHolder::onPlayerDashDecorations(frameContext* fc, const entity entityPlayer, const bool facingRight) {
	spawnSmoke(fc, entityPlayer.get<positionComponent>()->position);
}
void rfct::decorationHolder::update(frameContext* ctx) {
	updateSmokes(ctx);
}
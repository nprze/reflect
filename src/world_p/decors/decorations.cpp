#include "decorations.h"
#include "renderer_p/debug/debug_draw.h"
#include "smokes.h"
#include "dash_kindlings.h"
#include "world_p/components.h"
#include "assets/serialize_structures/scene_serialize_data.h"
#include "death_anim.h"
#include "grass.h"

void rfct::decorationHolder::init(sceneSerializedData* serializeData, scene* parentScene) {
	initSmokeVars(parentScene);
	initKindlingsVars(parentScene);
	initGrassVars(parentScene);

	spawnTallGrass(parentScene, serializeData);
}

rfct::decorationHolder::~decorationHolder()
{
	cleanupSmokes();
	cleanupKindlings();
	cleanupGrass();
}

void rfct::decorationHolder::onPlayerDashDecorations(frameContext* fc, const entity entityPlayer, const bool facingRight) {
}
void rfct::decorationHolder::update(frameContext* ctx) {
	RFCT_PROFILE_SCOPE("decorations update");

	updateSmokes(ctx);
	updateKindlings(ctx);
	updateGrass(ctx);

	updateSmokeMatrices(ctx);
	updateKindlingMatrices(ctx);
}
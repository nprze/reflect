#include "ui.h"
#include <string>
#include "input.h"
#include "renderer_p/renderer.h"

rfct::gameState beforePauseMenuState = rfct::gameState::gameplay;
rfct::gameState lastState = rfct::gameState::gameplay;
float timeSinceStateChange = 0.f;
glm::vec2 imageExtent = glm::vec2(0, 0);
float globalTime = 0.f;
uint32_t selectedMenuIndex = 0;
float changeSelectionCooldown = 0.f;
float multiplier = 1.f;

// UI aligned
namespace rfct {
	float totalHeight = 0.0f;
	float interline = 0.003f;
	float globalOffsetNorm = 0.f;
	struct UIAlignedTextInfo {
		std::string text;
		glm::vec3 color;
		float height = 0.f;
		float width = 0.f;
	};
	uint32_t lastAlignedTextInfoIndex = 0;
	UIAlignedTextInfo AlignedTextInfos[8];
	void addTextCenteredHV(const std::string& text, const glm::vec3& color, float heightNormalized) { //horizontally and vertically
		AlignedTextInfos[lastAlignedTextInfoIndex].text = text;
		AlignedTextInfos[lastAlignedTextInfoIndex].color = color;
		AlignedTextInfos[lastAlignedTextInfoIndex].height = heightNormalized * imageExtent.y;
		AlignedTextInfos[lastAlignedTextInfoIndex].width = renderer::getRen().getUIPipeline().getDefaultFont()->getTextWidth(text, heightNormalized * imageExtent.y);
		totalHeight += (heightNormalized + interline) * imageExtent.y;
		lastAlignedTextInfoIndex++;
	}
	void flushAlignedTextInfos() {
		totalHeight -= interline * imageExtent.y;
		float cursorY = 0.5f * imageExtent.y - (0.5f * totalHeight);
		for (uint32_t i = 0; i < lastAlignedTextInfoIndex; i++) {
			renderer::getRen().getUIPipeline().addTextVerticesHeight(
				AlignedTextInfos[i].text, 
				glm::vec2(
					(0.5f) * imageExtent.x - 0.5f * AlignedTextInfos[i].width,
					cursorY + (globalOffsetNorm * imageExtent.y)),
				AlignedTextInfos[i].height, 
				AlignedTextInfos[i].color);
			cursorY += interline * imageExtent.y + AlignedTextInfos[i].height;
		}
		lastAlignedTextInfoIndex = 0;
		totalHeight = 0.f;
	}
}

rfct::gameState rfct::getState()
{
	if (input::getInput().openClosePauseMenu && timeSinceStateChange>1.f) {
		timeSinceStateChange = 0.f;
		selectedMenuIndex = 0;
		if (lastState == gameState::menu) {
			lastState = beforePauseMenuState;
		}
		else {
			beforePauseMenuState = lastState;
			lastState = gameState::menu;
		}
	}
	return lastState;
}

void rfct::updateLastState(gameState newState)
{
	lastState = newState;
}

void rfct::drawUI(frameContext* ctx)
{
	timeSinceStateChange += ctx->dt;
	globalTime += ctx->dt;
	changeSelectionCooldown = std::clamp(changeSelectionCooldown - ctx->dt, 0.f, 0.25f);
	imageExtent = { static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().width),
					static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().height) };

	if (ctx->state == gameState::menu) {
		if (input::getInput().upDownMenu != 0 && changeSelectionCooldown <= 0.f) {
			selectedMenuIndex = (selectedMenuIndex - (uint32_t)input::getInput().upDownMenu + 3) % 3;
			multiplier = -input::getInput().upDownMenu;
			changeSelectionCooldown = 0.25f;
		}
		rfct::renderer::getRen().getUIPipeline().beginAddingTriangles();
		rfct::renderer::getRen().getUIPipeline().addTriangleNormalized({ 0,0 }, { 1, 0 }, { 0, 1 }, {0,0,0}, rfct::opacity::opacity75percent);
		rfct::renderer::getRen().getUIPipeline().addTriangleNormalized({ 1,1 }, { 1, 0 }, { 0, 1 }, {0,0,0}, rfct::opacity::opacity75percent);
		rfct::renderer::getRen().getUIPipeline().endAddingTriangles();
		float intensity = glm::sin(globalTime * 3.f) * 0.2f + .8f;
		addTextCenteredHV("RESUME",		(selectedMenuIndex == 0 ? 1.f: 0.5f) * glm::vec3{ intensity, intensity, intensity }, 0.07f);
		addTextCenteredHV("SETTINGS",	(selectedMenuIndex == 1 ? 1.f: 0.5f) * glm::vec3{ intensity, intensity, intensity }, 0.07f);
		addTextCenteredHV("QUIT",		(selectedMenuIndex == 2 ? 1.f: 0.5f) * glm::vec3{ intensity, intensity, intensity }, 0.07f);
		if (changeSelectionCooldown != 0) {
			globalOffsetNorm = glm::pow(-4*changeSelectionCooldown, 2);
			globalOffsetNorm *= 0.01f * multiplier;
		}
		else {
			globalOffsetNorm = 0.f;
		}
		flushAlignedTextInfos();
	}
	debugDraw::drawText("dt: " + std::to_string(ctx->dt) + "s", glm::vec2(0, 0), 0.08f);
}


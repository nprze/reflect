#include "ui.h"
#include <string>
#include "input.h"
#include "renderer_p/renderer.h"
#include "world_p/progress/user_progress.h"

rfct::gameState beforePauseMenuState = rfct::gameState::gameplay;
rfct::gameState lastState = rfct::gameState::gameplay;
float timeSinceStateChange = 0.f;
glm::vec2 imageExtent = glm::vec2(0, 0);
float globalTime = 0.f;

uint32_t currentMenuElementSelectedIndex = 0;
uint32_t currentMenuMaxElementIndex = 0;
float changeSelectionCooldown = 0.f;
float upDownEffectMultiplier = 1.f;

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

// UI decorations	
namespace rfct {
	struct triangleDecoration {
		glm::vec2 pos0;
		glm::vec2 pos1;
		glm::vec2 pos2;
		glm::vec2 pos;
		glm::vec2 dir;
		glm::vec3 color;
		float angle;
	};
	std::vector<triangleDecoration> triangleDecorations;
	void defineDecors(uint32_t count) {
		triangleDecorations.clear();
		triangleDecorations.reserve(count);
		for (uint32_t i = 0; i < count; i++) {
			triangleDecoration decor;
			decor.pos0 = glm::vec2(0.00f, 0.00f) * (static_cast<float>(rand() % 1000) / 2000.f + 0.5f);
			decor.pos1 = glm::vec2(0.02f, 0.02f) * (static_cast<float>(rand() % 1000) / 2000.f + 0.5f);
			decor.pos2 = glm::vec2(0.02f, -0.02f) * (static_cast<float>(rand() % 1000) / 2000.f + 0.5f);
			decor.pos = glm::vec2(
				static_cast<float>(rand() % 1000) / 1000.f,
				static_cast<float>(rand() % 1000) / 1000.f
			);
			float intensity = 0.1f + 0.5f * static_cast<float>(rand() % 2000) / 2000.f;
			decor.color = glm::vec3(intensity, intensity, intensity);
			decor.angle = static_cast<float>(rand() % 360);
			decor.dir = glm::normalize(glm::vec2(
				static_cast<float>(rand() % 2000) / 1000.f - 1.f,
				static_cast<float>(rand() % 2000) / 1000.f - 1.f
			));
			triangleDecorations.push_back(decor);
		}
	}
	void updateDecors(frameContext* ctx) {
		float aspectRatio = imageExtent.x / imageExtent.y;
		for (triangleDecoration& decor : triangleDecorations) {
			decor.angle += ctx->dt * 20.f;
			glm::mat2 rotationMatrix = glm::mat2(
				glm::vec2(glm::cos(glm::radians(decor.angle)), -glm::sin(glm::radians(decor.angle))),
				glm::vec2(glm::sin(glm::radians(decor.angle)), glm::cos(glm::radians(decor.angle)))
			);
			float additionalBoost = 0.f;
			if (changeSelectionCooldown != 0) {
				additionalBoost = glm::pow(-4 * changeSelectionCooldown, 2);
				additionalBoost *= -0.004f * upDownEffectMultiplier;
			}
			decor.pos.y += additionalBoost;

			decor.pos += decor.dir * ctx->dt * .05f;
			decor.dir = glm::normalize(decor.dir + glm::vec2(
				(static_cast<float>(rand() % 2000) / 1000.f - 1.f) * ctx->dt * 4.f,
				(static_cast<float>(rand() % 2000) / 1000.f - 1.f) * ctx->dt * 4.f
			));
			if (decor.pos.x < -0.1f) decor.pos.x = 1.1f;
			if (decor.pos.x > 1.1f) decor.pos.x = -0.1f;
			if (decor.pos.y < -0.1f) decor.pos.y = 1.1f;
			if (decor.pos.y > 1.1f) decor.pos.y = -0.1f;

			glm::vec2 p0 = (rotationMatrix * decor.pos0 + decor.pos) * glm::vec2{1, aspectRatio};
			glm::vec2 p1 = (rotationMatrix * decor.pos1 + decor.pos) * glm::vec2{1, aspectRatio};
			glm::vec2 p2 = (rotationMatrix * decor.pos2 + decor.pos) * glm::vec2{1, aspectRatio};

			rfct::renderer::getRen().getUIPipeline().addTriangleNormalized(p0, p1, p2, decor.color, opacity::opacity100percent);
		}
	}
	void cleanupDecors() {
		triangleDecorations.clear();
	}
}

// UI logic
namespace rfct {
	enum uiLogicState {
		basePauseMenu,
		baseSettings,
		soundSettings,
	};
	uiLogicState currentUILogicState = uiLogicState::basePauseMenu;
}

// menu specific update
namespace rfct {
	void switchMenu(uiLogicState newState) {
		currentUILogicState = newState;
		currentMenuElementSelectedIndex = 0;
	}
	void updateBaseMenu(frameContext* ctx) {
		if (currentMenuElementSelectedIndex == 0) { // resume
			timeSinceStateChange = 0.f;
			ctx->state = beforePauseMenuState;
		}
		else if (currentMenuElementSelectedIndex == 1) { // settings
			switchMenu(uiLogicState::baseSettings);
			currentMenuMaxElementIndex = 4;
		}
		else if (currentMenuElementSelectedIndex == 2) { // quit
			glfwSetWindowShouldClose(rfct::renderer::getRen().getWindow().GetHandle(), true);
		}
	}
	void updateBaseSettings(frameContext* ctx) {
		if (currentMenuElementSelectedIndex == 0) { // video
		}
		else if (currentMenuElementSelectedIndex == 1) { // sound
		}
		else if (currentMenuElementSelectedIndex == 2) { // controls
		}
		else if (currentMenuElementSelectedIndex == 3) { // back
			userSettings::get().dumpUserSettings();
			switchMenu(uiLogicState::basePauseMenu);
			currentMenuMaxElementIndex = 3;
		}
	}
}

// draw functions
namespace rfct {
	void drawBaseMenu(frameContext* ctx) {
		// text
		if (changeSelectionCooldown != 0) {
			globalOffsetNorm = glm::pow(-4 * changeSelectionCooldown, 2);
			globalOffsetNorm *= 0.01f * upDownEffectMultiplier;
		}
		else {
			globalOffsetNorm = 0.f;
		}
		float intensity = glm::sin(globalTime * 3.f) * 0.2f + .8f;
		addTextCenteredHV("RESUME", (currentMenuElementSelectedIndex == 0 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, 0.07f);
		addTextCenteredHV("SETTINGS", (currentMenuElementSelectedIndex == 1 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, 0.07f);
		addTextCenteredHV("SAVE AND QUIT", (currentMenuElementSelectedIndex == 2 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, 0.07f);
		flushAlignedTextInfos();
	}
	void drawBaseSettings(frameContext* ctx) {
		// text
		if (changeSelectionCooldown != 0) {
			globalOffsetNorm = glm::pow(-4 * changeSelectionCooldown, 2);
			globalOffsetNorm *= 0.01f * upDownEffectMultiplier;
		}
		else {
			globalOffsetNorm = 0.f;
		}
		float intensity = glm::sin(globalTime * 3.f) * 0.2f + .8f;
		addTextCenteredHV("VIDEO", (currentMenuElementSelectedIndex == 0 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, 0.07f);
		addTextCenteredHV("SOUND", (currentMenuElementSelectedIndex == 1 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, 0.07f);
		addTextCenteredHV("CONTROLS", (currentMenuElementSelectedIndex == 2 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, 0.07f);
		addTextCenteredHV("BACK", (currentMenuElementSelectedIndex == 3 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, 0.07f);
		flushAlignedTextInfos();
	}
}

rfct::gameState rfct::getState()
{
	if (input::getInput().openClosePauseMenu && timeSinceStateChange>1.f) {
		timeSinceStateChange = 0.f;
		currentMenuElementSelectedIndex = 0;
		if (lastState == gameState::menu) {
			lastState = beforePauseMenuState;
			cleanupDecors();
		}
		else {
			beforePauseMenuState = lastState;
			lastState = gameState::menu;
			currentUILogicState = uiLogicState::basePauseMenu;
			currentMenuMaxElementIndex = 3;
			defineDecors(5);
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
	// general updates
	timeSinceStateChange += ctx->dt;
	globalTime += ctx->dt;
	changeSelectionCooldown = std::clamp(changeSelectionCooldown - ctx->dt, 0.f, 0.25f);
	imageExtent = { static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().width),
					static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().height) };

	// text and button draw
	if (ctx->state == gameState::menu) {
		// input updates
		if (input::getInput().upDownMenu != 0 && changeSelectionCooldown <= 0.f) {
			currentMenuElementSelectedIndex = (currentMenuElementSelectedIndex - (uint32_t)input::getInput().upDownMenu + currentMenuMaxElementIndex) % currentMenuMaxElementIndex;
			upDownEffectMultiplier = -input::getInput().upDownMenu;
			changeSelectionCooldown = 0.25f;
		}

		// background draw
		rfct::renderer::getRen().getUIPipeline().beginAddingTriangles();
		rfct::renderer::getRen().getUIPipeline().addTriangleNormalized({ 0,0 }, { 2, 0 }, { 0, 2 }, { 0,0,0 }, rfct::opacity::opacity75percent);
		rfct::renderer::getRen().getUIPipeline().addTriangleNormalized({ 2,2 }, { 2, 0 }, { 0, 2 }, { 0,0,0 }, rfct::opacity::opacity75percent);
		updateDecors(ctx);
		rfct::renderer::getRen().getUIPipeline().endAddingTriangles();

		switch (currentUILogicState)
		{
		case rfct::basePauseMenu: {
			drawBaseMenu(ctx);
			break;
		}
		case rfct::baseSettings: {
			drawBaseSettings(ctx);
			break;
		}
		case rfct::soundSettings:
			break;
		default:
			break;
		}
	}

	if (input::getInput().selectMenu && changeSelectionCooldown <= 0.f) {
		changeSelectionCooldown = 0.25f;
		switch (currentUILogicState)
		{
		case rfct::basePauseMenu: {
			updateBaseMenu(ctx);
			break;
		}
		case rfct::baseSettings: {
			updateBaseSettings(ctx);
			break;
		}
		case rfct::soundSettings:
			break;
		default:
			break;
		}
	}
	int fps = static_cast<int>(std::floor(1.0 / ctx->dt));
	debugDraw::drawText("fps: " + std::to_string(fps), glm::vec2(0, 0), 0.08f);
}


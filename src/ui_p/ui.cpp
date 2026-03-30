#include "ui.h"
#include <string>
#include "input.h"
#include "renderer_p/renderer.h"
#include "world_p/progress/user_progress.h"

using updateFunc = void(*)(rfct::frameContext*, bool select, float leftRight);
using drawFunc = void(*)(rfct::frameContext*);

enum UIPartsNames {
	UIPartsNames_basePauseMenu,
	UIPartsNames_baseSettings,
	UIPartsNames_soundSettings,
};

struct UIPart {
	drawFunc drawFunction;
	updateFunc updateFunction;
	uint32_t elementsCount;
};
// Parts/ menus
std::map<UIPartsNames, UIPart> uiParts;
UIPartsNames currentUIPart = UIPartsNames::UIPartsNames_basePauseMenu;

// among parts variables
rfct::gameState beforePauseMenuState = rfct::gameState::gameplay;
rfct::gameState lastState = rfct::gameState::gameplay;
float timeSinceStateChange = 0.f;

// utilites/ helpers
glm::vec2 imageExtent = glm::vec2(0, 0);
float globalTime = 0.f;

// specific parts variables
uint32_t currentMenuElementSelectedIndex = 0;
float changeSelectionCooldown = 0.f;
float upDownEffectMultiplier = 1.f;

// UI aligned
namespace rfct {
	enum HorizontalAlignment {
		HorizontalAignment_right,
		HorizontalAignment_left,
		HorizontalAignment_middle
	};
	float totalHeight = 0.0f;
	float interline = 0.003f;
	float globalOffsetNorm = 0.f; // <0,1>
	float globalAlignmentStartNorm = 0.5f; // <0,1>, when the horizontal alignment is set to left and this is set to 0.6, it starts drawing from 0.2 of whole image width (same for right)
	struct UIAlignedTextInfo {
		std::string text;
		glm::vec3 color;
		float height = 0.f;
		float width = 0.f;
		HorizontalAlignment horizontalAlignment;
	};
	uint32_t lastAlignedTextInfoIndex = 0;
	UIAlignedTextInfo AlignedTextInfos[8];
	void addTextCenteredVertical(const std::string& text, const glm::vec3& color, HorizontalAlignment alignment, float heightNormalized = 0.07f) {
		AlignedTextInfos[lastAlignedTextInfoIndex].text = text;
		AlignedTextInfos[lastAlignedTextInfoIndex].color = color;
		AlignedTextInfos[lastAlignedTextInfoIndex].height = heightNormalized * imageExtent.y;
		AlignedTextInfos[lastAlignedTextInfoIndex].width = renderer::getRen().getUIPipeline().getDefaultFont()->getTextWidth(text, heightNormalized * imageExtent.y);
		AlignedTextInfos[lastAlignedTextInfoIndex].horizontalAlignment = alignment;
		totalHeight += (heightNormalized + interline) * imageExtent.y;
		lastAlignedTextInfoIndex++;
	}
	void flushAlignedTextInfos() {
		totalHeight -= interline * imageExtent.y;
		float cursorY = 0.5f * imageExtent.y - (0.5f * totalHeight);
		for (uint32_t i = 0; i < lastAlignedTextInfoIndex; i++) {
			float xStart = 0.0f;
			if (AlignedTextInfos[i].horizontalAlignment == HorizontalAignment_middle) {
				xStart = (0.5f) * imageExtent.x - 0.5f * AlignedTextInfos[i].width;
			}
			else if (AlignedTextInfos[i].horizontalAlignment == HorizontalAignment_left) {
				xStart = (0.5f * (1 - globalAlignmentStartNorm)) * imageExtent.x;
			}
			else if (AlignedTextInfos[i].horizontalAlignment == HorizontalAignment_right) {
				xStart = (0.5f + (0.5f * globalAlignmentStartNorm)) * imageExtent.x - AlignedTextInfos[i].width;
			}
			renderer::getRen().getUIPipeline().addTextVerticesHeight(
				AlignedTextInfos[i].text, 
				glm::vec2(
					xStart,
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

// menu specific update
namespace rfct {
	void updateBaseMenu(frameContext* ctx, bool select, float leftRight) {
		if (!select) return;
		if (currentMenuElementSelectedIndex == 0) { // resume
			timeSinceStateChange = 0.f;
			ctx->state = beforePauseMenuState;
		}
		else if (currentMenuElementSelectedIndex == 1) { // settings
			switchUIPart(UIPartsNames_baseSettings);
		}
		else if (currentMenuElementSelectedIndex == 2) { // quit
			glfwSetWindowShouldClose(rfct::renderer::getRen().getWindow().GetHandle(), true);
		}
	}
	void updateBaseSettings(frameContext* ctx, bool select, float leftRight) {
		if (!select) return;
		if (currentMenuElementSelectedIndex == 0) { // video
		}
		else if (currentMenuElementSelectedIndex == 1) { // sound
			userSettings::get().dumpUserSettings();
			switchUIPart(UIPartsNames_soundSettings);
		}
		else if (currentMenuElementSelectedIndex == 2) { // controls
		}
		else if (currentMenuElementSelectedIndex == 3) { // back
			userSettings::get().dumpUserSettings();
			switchUIPart(UIPartsNames_basePauseMenu);
		}
	}
	void updateSoundSettings(frameContext* ctx, bool select, float leftRight) {
		if (currentMenuElementSelectedIndex == 0 && leftRight != 0) { // master volume
			userSettings::get().seriaizeData.masterVoicePercentage = std::clamp(userSettings::get().seriaizeData.masterVoicePercentage + (10 * leftRight), 0.f, 100.f);
			RFCT_INFO("master volume changed {}", userSettings::get().seriaizeData.masterVoicePercentage);
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
		addTextCenteredVertical("RESUME", (currentMenuElementSelectedIndex == 0 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, HorizontalAignment_middle);
		addTextCenteredVertical("SETTINGS", (currentMenuElementSelectedIndex == 1 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, HorizontalAignment_middle);
		addTextCenteredVertical("SAVE AND QUIT", (currentMenuElementSelectedIndex == 2 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, HorizontalAignment_middle);
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
		addTextCenteredVertical("VIDEO", (currentMenuElementSelectedIndex == 0 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, HorizontalAignment_middle);
		addTextCenteredVertical("SOUND", (currentMenuElementSelectedIndex == 1 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, HorizontalAignment_middle);
		addTextCenteredVertical("CONTROLS", (currentMenuElementSelectedIndex == 2 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, HorizontalAignment_middle);
		addTextCenteredVertical("BACK", (currentMenuElementSelectedIndex == 3 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, HorizontalAignment_middle);
		flushAlignedTextInfos();
	}
	void drawSoundSettings(frameContext* ctx) {
		float intensity = glm::sin(globalTime * 3.f) * 0.2f + .8f;
		addTextCenteredVertical("MASTER VOLUME", (currentMenuElementSelectedIndex == 1 ? 1.f : 0.5f) * glm::vec3{ intensity, intensity, intensity }, HorizontalAignment_left);
		std::string currentVolume = std::to_string(int(userSettings::get().seriaizeData.masterVoicePercentage));
		addTextCenteredVertical(currentVolume.c_str(), (currentMenuElementSelectedIndex == 1 ? 1.f : 0.5f) * glm::vec3{intensity, intensity, intensity}, HorizontalAignment_right);
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
			switchUIPart(UIPartsNames_basePauseMenu);
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
			uint32_t elementCount = uiParts[currentUIPart].elementsCount;
			currentMenuElementSelectedIndex = (currentMenuElementSelectedIndex - (uint32_t)input::getInput().upDownMenu + elementCount) % elementCount;
			upDownEffectMultiplier = -input::getInput().upDownMenu;
			changeSelectionCooldown = 0.25f;
		}

		// background draw
		rfct::renderer::getRen().getUIPipeline().beginAddingTriangles();
		rfct::renderer::getRen().getUIPipeline().addTriangleNormalized({ 0,0 }, { 2, 0 }, { 0, 2 }, { 0,0,0 }, rfct::opacity::opacity75percent);
		rfct::renderer::getRen().getUIPipeline().addTriangleNormalized({ 2,2 }, { 2, 0 }, { 0, 2 }, { 0,0,0 }, rfct::opacity::opacity75percent);
		updateDecors(ctx);
		rfct::renderer::getRen().getUIPipeline().endAddingTriangles();

		uiParts[currentUIPart].drawFunction(ctx);
	}

	if ((input::getInput().selectMenu || input::getInput().leftRightMenu != 0) && changeSelectionCooldown <= 0.f) {
		changeSelectionCooldown = 0.25f;
		uiParts[currentUIPart].updateFunction(ctx, input::getInput().selectMenu, input::getInput().leftRightMenu);
	}
	int fps = static_cast<int>(std::floor(1.0 / ctx->dt));
	debugDraw::drawText("fps: " + std::to_string(fps), glm::vec2(0, 0), 0.08f);
}

void rfct::switchUIPart(UIPartsNames part)
{
	currentUIPart = part;
	currentMenuElementSelectedIndex = 0;
}

void rfct::defineUI()
{
	uiParts[UIPartsNames_basePauseMenu] = { drawBaseMenu, updateBaseMenu, 3 };
	uiParts[UIPartsNames_baseSettings] = { drawBaseSettings, updateBaseSettings, 4 };
	uiParts[UIPartsNames_soundSettings] = { drawSoundSettings, updateSoundSettings, 1 };
}

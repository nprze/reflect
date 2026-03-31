#include "ui.h"
#include <string>
#include "input.h"
#include "renderer_p/renderer.h"
#include "world_p/progress/user_progress.h"

namespace rfct {
	using ActionFunction = void(*)(rfct::frameContext*);
	// FAT STRUCT
	struct UINode {
		std::string label;
		enum UINodeType {
			UINodeType_Menu,
			UINodeType_IntVariable,
			UINodeType_ActionButton
		} type;
		// menu specific
		uint32_t previousIndex = 0;
		uint32_t childrenIndices[8];
		uint32_t childrenCount = 0;
		// int var specific
		uint32_t minValue;
		uint32_t maxValue;
		uint32_t* valuePtr;
		// action button specific
		ActionFunction action;
	};
}

// display settings
constexpr float textScale = 0.07f;
constexpr float interline = 0.003f;
constexpr float globalAlignmentStartNorm = 0.5f; // <0,1>, when the horizontal alignment is set to left and this is set to 0.6, it starts drawing from 0.2 of whole image width (same for right)

// UI structure
rfct::UINode UINodes[16];
uint32_t currentNodeIndex = 0;

// state
rfct::gameState beforePauseMenuState = rfct::gameState::gameplay;
rfct::gameState lastState = rfct::gameState::gameplay;
float timeSinceStateChange = 0.f;

// helpers
glm::vec2 imageExtent = glm::vec2(0, 0);
float globalTime = 0.f;

// animation and selection variables
uint32_t currentMenuElementSelectedIndex = 0;
float changeSelectionCooldown = 0.f;
float upDownEffectMultiplier = 1.f;

// actions
void actionResume(rfct::frameContext* ctx) {
	timeSinceStateChange = 0.f;
	ctx->state = beforePauseMenuState;
	currentNodeIndex = 0;
}

/*
// UI aligned
namespace rfct {
	enum HorizontalAlignment {
		HorizontalAignment_right,
		HorizontalAignment_left,
		HorizontalAignment_middle
	};
	struct UIAlignedTextInfo {
		std::string text;
		glm::vec3 color;
		float height = 0.f;
		float width = 0.f;
		HorizontalAlignment horizontalAlignment;
	};

	constexpr float textScale = 0.07f;
	constexpr float interline = 0.003f;
	constexpr float globalAlignmentStartNorm = 0.5f; // <0,1>, when the horizontal alignment is set to left and this is set to 0.6, it starts drawing from 0.2 of whole image width (same for right)
	
	float animationOffsetNorm = 0.f; // <0,1>
	uint32_t alignedTextLineCount = 0;
	uint32_t lastAlignedTextInfoIndex = 0;
	UIAlignedTextInfo AlignedTextInfos[16];

	void addTextCenteredVertical(const std::string& text, const glm::vec3& color, HorizontalAlignment alignment) {
		AlignedTextInfos[lastAlignedTextInfoIndex].text = text;
		AlignedTextInfos[lastAlignedTextInfoIndex].color = color;
		AlignedTextInfos[lastAlignedTextInfoIndex].height = textScale * imageExtent.y;
		AlignedTextInfos[lastAlignedTextInfoIndex].width = renderer::getRen().getUIPipeline().getDefaultFont()->getTextWidth(text, textScale * imageExtent.y);
		AlignedTextInfos[lastAlignedTextInfoIndex].horizontalAlignment = alignment;
		alignedTextLineCount++;
		lastAlignedTextInfoIndex++;
	}
	void flushAlignedTextInfos() {
		float totalHeight = ((alignedTextLineCount * textScale) + ((alignedTextLineCount - 1) * interline)) * imageExtent.y;
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
					cursorY + (animationOffsetNorm * imageExtent.y)),
				AlignedTextInfos[i].height, 
				AlignedTextInfos[i].color);
			cursorY += interline * imageExtent.y + AlignedTextInfos[i].height;
		}
		lastAlignedTextInfoIndex = 0;
		alignedTextLineCount = 0;
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


void rfct::switchUIPart(UIPartsNames part)
{
	currentUIPart = part;
	currentMenuElementSelectedIndex = 0;
}

namespace rfct {
	void drawBaseMenu(frameContext* ctx, float colorIntensity) {
		addTextCenteredVertical("RESUME", (currentMenuElementSelectedIndex == 0 ? 1.f : 0.5f) * glm::vec3{ colorIntensity, colorIntensity, colorIntensity }, HorizontalAignment_middle);
		addTextCenteredVertical("SETTINGS", (currentMenuElementSelectedIndex == 1 ? 1.f : 0.5f) * glm::vec3{ colorIntensity, colorIntensity, colorIntensity }, HorizontalAignment_middle);
		addTextCenteredVertical("SAVE AND QUIT", (currentMenuElementSelectedIndex == 2 ? 1.f : 0.5f) * glm::vec3{ colorIntensity, colorIntensity, colorIntensity }, HorizontalAignment_middle);
		flushAlignedTextInfos();
	}
	void drawBaseSettings(frameContext* ctx, float colorIntensity) {
		addTextCenteredVertical("VIDEO", (currentMenuElementSelectedIndex == 0 ? 1.f : 0.5f) * glm::vec3{ colorIntensity, colorIntensity, colorIntensity }, HorizontalAignment_middle);
		addTextCenteredVertical("SOUND", (currentMenuElementSelectedIndex == 1 ? 1.f : 0.5f) * glm::vec3{ colorIntensity, colorIntensity, colorIntensity }, HorizontalAignment_middle);
		addTextCenteredVertical("CONTROLS", (currentMenuElementSelectedIndex == 2 ? 1.f : 0.5f) * glm::vec3{ colorIntensity, colorIntensity, colorIntensity }, HorizontalAignment_middle);
		addTextCenteredVertical("BACK", (currentMenuElementSelectedIndex == 3 ? 1.f : 0.5f) * glm::vec3{ colorIntensity, colorIntensity, colorIntensity }, HorizontalAignment_middle);
		flushAlignedTextInfos();
	}
	void drawSoundSettings(frameContext* ctx, float colorIntensity) {
		addTextCenteredVertical("MASTER VOLUME", (currentMenuElementSelectedIndex == 1 ? 1.f : 0.5f) * glm::vec3{ colorIntensity, colorIntensity, colorIntensity }, HorizontalAignment_left);
		std::string currentVolume = std::to_string(int(userSettings::get().seriaizeData.masterVoicePercentage));
		addTextCenteredVertical(currentVolume.c_str(), (currentMenuElementSelectedIndex == 1 ? 1.f : 0.5f) * glm::vec3{ colorIntensity, colorIntensity, colorIntensity }, HorizontalAignment_right);
		flushAlignedTextInfos();
	}
}*/

void rfct::drawUI(frameContext* ctx)
{
	// general updates
	timeSinceStateChange += ctx->dt;
	globalTime += ctx->dt;
	changeSelectionCooldown = std::clamp(changeSelectionCooldown - ctx->dt, 0.f, 0.25f);

	int fps = static_cast<int>(std::floor(1.0 / ctx->dt));
	debugDraw::drawText("fps: " + std::to_string(fps), glm::vec2(0, 0), 0.08f);
	if (ctx->state != gameState::menu) return;

	// helper
	imageExtent = { static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().width), static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().height) };

	// input updates
	if (input::getInput().upDownMenu != 0 && changeSelectionCooldown <= 0.f) {
		uint32_t elementCount = UINodes[currentNodeIndex].childrenCount;
		currentMenuElementSelectedIndex = (currentMenuElementSelectedIndex - (uint32_t)input::getInput().upDownMenu + elementCount) % elementCount;
		upDownEffectMultiplier = -input::getInput().upDownMenu;
		changeSelectionCooldown = 0.25f;
	}

	// background draw
	rfct::renderer::getRen().getUIPipeline().beginAddingTriangles();
	rfct::renderer::getRen().getUIPipeline().addTriangleNormalized({ 0,0 }, { 2, 0 }, { 0, 2 }, { 0,0,0 }, rfct::opacity::opacity75percent);
	rfct::renderer::getRen().getUIPipeline().addTriangleNormalized({ 2,2 }, { 2, 0 }, { 0, 2 }, { 0,0,0 }, rfct::opacity::opacity75percent);
	//updateDecors(ctx);
	rfct::renderer::getRen().getUIPipeline().endAddingTriangles();

	if (changeSelectionCooldown != 0) {
		animationOffsetNorm = glm::pow(-4 * changeSelectionCooldown, 2);
		animationOffsetNorm *= 0.01f * upDownEffectMultiplier;
	}
	else {
		animationOffsetNorm = 0.f;
	}

	float intensity = glm::sin(globalTime * 3.f) * 0.2f + .8f;
	uiParts[currentUIPart].drawFunction(ctx, intensity);

	if ((input::getInput().selectMenu || input::getInput().leftRightMenu != 0) && changeSelectionCooldown <= 0.f) {
		changeSelectionCooldown = 0.25f;
		uiParts[currentUIPart].updateFunction(ctx, input::getInput().selectMenu, input::getInput().leftRightMenu);
	}
}


void rfct::defineUI()
{
	UINodes[0].label = std::string("PAUSE MENU");
	UINodes[0].type = UINode::UINodeType::UINodeType_Menu;
	UINodes[0].previousIndex = -1;
	UINodes[0].childrenIndices[0] = 1;
	UINodes[0].childrenIndices[1] = 2;
	UINodes[0].childrenIndices[2] = 3;
	UINodes[0].childrenCount = 3;

	UINodes[1].label = std::string("RESUME 0");
	UINodes[1].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[1].action = actionResume;

	UINodes[2].label = std::string("RESUME 1");
	UINodes[2].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[2].action = actionResume;

	UINodes[3].label = std::string("RESUME 2");
	UINodes[3].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[3].action = actionResume;
}

#include "ui.h"
#include <string>
#include <stack>
#include "input.h"
#include "renderer_p/renderer.h"
#include "world_p/progress/user_progress.h"

namespace rfct {
	using ActionFunction = void(*)(rfct::frameContext*);
	// FAT STRUCT
	struct UINode {
		std::string label;
		glm::vec3 color = {1.f, 1.f, 1.f};
		enum UINodeType {
			UINodeType_Menu,
			UINodeType_IntPercentage,
			UINodeType_ActionButton
		} type;
		// menu specific
		uint32_t previousIndex = 0;
		uint32_t childrenIndices[7];
		uint32_t childrenCount = 0;
		// int var specific
		uint32_t minValue;
		uint32_t maxValue;
		uint32_t step;
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
std::stack<uint32_t> previousNodeIndices;

// state
rfct::gameState beforePauseMenuState = rfct::gameState::gameplay;
rfct::gameState lastState = rfct::gameState::gameplay;
float timeSinceStateChange = 0.f;

// helpers
glm::vec2 imageExtent = glm::vec2(0, 0);
float globalTime = 0.f;

// animation and selection variables
uint32_t currentSelectedMenuIndex = 0;
float changeSelectionCooldown = 0.f;
float upDownEffectMultiplier = 1.f;
float animationOffsetNorm = 0.f; // <0,1>

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

			glm::vec2 p0 = (rotationMatrix * decor.pos0 + decor.pos) * glm::vec2{ 1, aspectRatio };
			glm::vec2 p1 = (rotationMatrix * decor.pos1 + decor.pos) * glm::vec2{ 1, aspectRatio };
			glm::vec2 p2 = (rotationMatrix * decor.pos2 + decor.pos) * glm::vec2{ 1, aspectRatio };

			rfct::renderer::getRen().getUIPipeline().addTriangleNormalized(p0, p1, p2, decor.color, opacity::opacity100percent);
		}
	}
	void cleanupDecors() {
		triangleDecorations.clear();
	}
}

// actions
void actionResume(rfct::frameContext* ctx) {
	timeSinceStateChange = 0.f;
	ctx->state = beforePauseMenuState;
	currentNodeIndex = -1;
	while (!previousNodeIndices.empty()) previousNodeIndices.pop();
	rfct::cleanupDecors();
}
void actionQuit(rfct::frameContext* ctx) {
	rfct::userSettings::get().dumpUserSettings();
	glfwSetWindowShouldClose(rfct::renderer::getRen().getWindow().GetHandle(), true);
}
void actionProgressDeveloper(rfct::frameContext* ctx) {
	static uint32_t progress = 0;
	progress += 1;
	if (progress >= 7) {
		UINodes[9].label = "You are now a developer.";
		if (rfct::userSettings::get().seriaizeData.isDeveloper == 0) {
			rfct::userSettings::get().seriaizeData.isDeveloper = 1;
			rfct::userSettings::get().dumpUserSettings();
			UINodes[0].childrenIndices[3] = 10;
			UINodes[0].childrenIndices[4] = 4;
			UINodes[0].childrenCount = 5;
		}
	}
	else if (progress > 3) {
		UINodes[9].label = "You are "+std::to_string(7 - progress)+" clicks away from being a developer.";
		UINodes[3].childrenIndices[1] = 9;
		UINodes[3].childrenCount = 2;
	}
}
void actionTellStory(rfct::frameContext* ctx) {
	static uint32_t storyProgress = 0;
	storyProgress += 1;
	if (storyProgress == 1)UINodes[11].label = "thank you for playing smokes.";
	else if (storyProgress == 2)UINodes[11].label = ":3";
	else if (storyProgress == 5) UINodes[11].label = ">:3";
	else if (storyProgress == 6) UINodes[11].label = ">:(";
	else if (storyProgress == 7) UINodes[11].label = "Please enjoy the game.";
	else if (storyProgress == 8) UINodes[11].label = "This is not a clicker.";
	else if (storyProgress == 9) UINodes[11].label = "There is nothing more here, I promise.";
}
void actionEmpty(rfct::frameContext* ctx) {
}

rfct::gameState rfct::getState()
{
	if (input::getInput().openClosePauseMenu && timeSinceStateChange > 1.f) {
		timeSinceStateChange = 0.f;
		currentSelectedMenuIndex = 0;
		if (lastState == gameState::menu) {
			lastState = beforePauseMenuState;
			currentNodeIndex = -1;
			while (!previousNodeIndices.empty()) previousNodeIndices.pop();
			cleanupDecors();
		}
		else {
			beforePauseMenuState = lastState;
			lastState = gameState::menu;
			currentNodeIndex = 0;
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

	int fps = static_cast<int>(std::floor(1.0 / ctx->dt));
	debugDraw::drawText("fps: " + std::to_string(fps), glm::vec2(0, 0), 0.07f);
	if (ctx->state != gameState::menu || currentNodeIndex == -1) return;

	// helper
	imageExtent = { static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().width), static_cast<float>(rfct::renderer::getRen().getRenderImagesManager().getSwapChain().getExtent().height) };
	bool hasBackButton = currentNodeIndex != 0;

	// input updates
	if (input::getInput().upDownMenu != 0 && changeSelectionCooldown <= 0.f) {
		uint32_t elementCount = UINodes[currentNodeIndex].childrenCount + (hasBackButton?1:0);
		currentSelectedMenuIndex = (currentSelectedMenuIndex - (uint32_t)input::getInput().upDownMenu + elementCount) % elementCount;
		upDownEffectMultiplier = -input::getInput().upDownMenu;
		changeSelectionCooldown = 0.25f;
	}

	// background draw
	rfct::renderer::getRen().getUIPipeline().beginAddingTriangles();
	rfct::renderer::getRen().getUIPipeline().addTriangleNormalized({ 0,0 }, { 2, 0 }, { 0, 2 }, { 0,0,0 }, rfct::opacity::opacity75percent);
	rfct::renderer::getRen().getUIPipeline().addTriangleNormalized({ 2,2 }, { 2, 0 }, { 0, 2 }, { 0,0,0 }, rfct::opacity::opacity75percent);
	updateDecors(ctx);
	rfct::renderer::getRen().getUIPipeline().endAddingTriangles();

	if (changeSelectionCooldown != 0) {
		animationOffsetNorm = glm::pow(-4 * changeSelectionCooldown, 2);
		animationOffsetNorm *= 0.01f * upDownEffectMultiplier;
	}
	else {
		animationOffsetNorm = 0.f;
	}

	// drawing
	float pulseColorIntensity = glm::sin(globalTime * 3.f) * 0.2f + .7f;
	float oneLineHeight = textScale * imageExtent.y;
	uint32_t elementCount = UINodes[currentNodeIndex].childrenCount + (hasBackButton ? 1 : 0);
	float totalHeight = UINodes[currentNodeIndex].childrenCount * oneLineHeight + (UINodes[currentNodeIndex].childrenCount - 1) * interline * imageExtent.y;
	float startY = 0.5f * imageExtent.y - (0.5f * totalHeight);
	font* defaultFont = renderer::getRen().getUIPipeline().getDefaultFont();

	for (uint32_t i = 0; i < UINodes[currentNodeIndex].childrenCount; i++) {
		uint32_t childIndex = UINodes[currentNodeIndex].childrenIndices[i];
		float width = defaultFont->getTextWidth(UINodes[childIndex].label, textScale * imageExtent.y);
		float selectionColorIntensity = (currentSelectedMenuIndex == i?1.f:0.5f);
		switch (UINodes[childIndex].type)
		{
		case UINode::UINodeType::UINodeType_ActionButton: 
		case UINode::UINodeType::UINodeType_Menu: {
			float xPos = (0.5f) * imageExtent.x - 0.5f * width;
			float yPos = startY + i * (oneLineHeight + interline * imageExtent.y);
			// draw text
			renderer::getRen().getUIPipeline().addTextVerticesHeight(
				UINodes[childIndex].label,
				glm::vec2(
					xPos,
					yPos + (animationOffsetNorm * imageExtent.y)),
				textScale * imageExtent.y,
				UINodes[childIndex].color * pulseColorIntensity * selectionColorIntensity);
			break;
		}
		case UINode::UINodeType::UINodeType_IntPercentage: {
			// label
			renderer::getRen().getUIPipeline().addTextVerticesHeight(
				UINodes[childIndex].label,
				glm::vec2(
					(0.15f) * imageExtent.x,
					startY + i * (oneLineHeight + interline * imageExtent.y) + (animationOffsetNorm * imageExtent.y)),
				textScale * imageExtent.y,
				UINodes[childIndex].color * pulseColorIntensity * selectionColorIntensity);

			// value
			std::string valueText = ((*UINodes[childIndex].valuePtr == UINodes[childIndex].minValue)?"":"< ") + std::to_string(*UINodes[childIndex].valuePtr) + ((*UINodes[childIndex].valuePtr == UINodes[childIndex].maxValue) ? "%" : "% >");
			float valueWidth = defaultFont->getTextWidth(valueText, textScale * imageExtent.y);
			renderer::getRen().getUIPipeline().addTextVerticesHeight(
				valueText,
				glm::vec2(
					(0.85f) * imageExtent.x - valueWidth,
					startY + i * (oneLineHeight + interline * imageExtent.y) + (animationOffsetNorm * imageExtent.y)),
				textScale * imageExtent.y,
				UINodes[childIndex].color * pulseColorIntensity * selectionColorIntensity);

			break;
		}
		}
	}
	if (elementCount - UINodes[currentNodeIndex].childrenCount == 1) {
		// draw back button
		float selectionColorIntensity = (currentSelectedMenuIndex == UINodes[currentNodeIndex].childrenCount ? 1.f : 0.5f);
		float width = defaultFont->getTextWidth("BACK", textScale * imageExtent.y);
		float xPos = (0.5f) * imageExtent.x - 0.5f * width;
		float yPos = startY + UINodes[currentNodeIndex].childrenCount * (oneLineHeight + interline * imageExtent.y);
		renderer::getRen().getUIPipeline().addTextVerticesHeight(
			std::string("BACK"),
			glm::vec2(
				xPos,
				yPos + (animationOffsetNorm * imageExtent.y)),
			textScale * imageExtent.y,
			glm::vec3(1.f, 1.f, 1.f) * pulseColorIntensity * selectionColorIntensity);
	}

	if ((input::getInput().selectMenu || input::getInput().leftRightMenu != 0) && changeSelectionCooldown <= 0.f) {
		changeSelectionCooldown = 0.25f;
		if (UINodes[currentNodeIndex].childrenCount == currentSelectedMenuIndex) {
			// back button selected
			userSettings::get().dumpUserSettings();
			currentNodeIndex = previousNodeIndices.top();
			previousNodeIndices.pop();
			currentSelectedMenuIndex = 0;
		}
		else {
			uint32_t selectedNodeIndex = UINodes[currentNodeIndex].childrenIndices[currentSelectedMenuIndex];
			if (input::getInput().selectMenu) {
				switch (UINodes[selectedNodeIndex].type) {
				case UINode::UINodeType::UINodeType_Menu: {
					previousNodeIndices.push(currentNodeIndex);
					currentNodeIndex = selectedNodeIndex;
					currentSelectedMenuIndex = 0;
					break;
				}
				case UINode::UINodeType::UINodeType_ActionButton: {
					UINodes[selectedNodeIndex].action(ctx);
					break;
				}
				}
			}
			else {
				switch (UINodes[selectedNodeIndex].type) {
				case UINode::UINodeType::UINodeType_IntPercentage: {
					*UINodes[selectedNodeIndex].valuePtr = (uint32_t)std::clamp(
						(int)((int)(*UINodes[selectedNodeIndex].valuePtr) + (int)input::getInput().leftRightMenu * UINodes[selectedNodeIndex].step),
						(int)UINodes[selectedNodeIndex].minValue,
						(int)UINodes[selectedNodeIndex].maxValue
					);
					break;
				}
				}
			}
		}
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
	UINodes[0].childrenIndices[3] = 4;
	UINodes[0].childrenCount = 4;
	if (userSettings::get().seriaizeData.isDeveloper) {
		UINodes[0].childrenIndices[3] = 10;
		UINodes[0].childrenIndices[4] = 4;
		UINodes[0].childrenCount = 5;
	}

	UINodes[1].label = std::string("RESUME");
	UINodes[1].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[1].action = actionResume;

	UINodes[2].label = std::string("SETTINGS");
	UINodes[2].type = UINode::UINodeType::UINodeType_Menu;
	UINodes[2].childrenIndices[0] = 5;
	UINodes[2].childrenIndices[1] = 6;
	UINodes[2].childrenCount = 2;

	UINodes[3].label = std::string("CREDITS");
	UINodes[3].type = UINode::UINodeType::UINodeType_Menu;
	UINodes[3].childrenIndices[0] = 8;
	UINodes[3].childrenCount = 1;

	UINodes[4].label = std::string("SAVE AND QUIT");
	UINodes[4].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[4].action = actionQuit;

	UINodes[5].label = std::string("GRAPHICS");
	UINodes[5].type = UINode::UINodeType::UINodeType_Menu;
	UINodes[5].childrenIndices[0] = 12;
	UINodes[5].childrenCount = 1;

	UINodes[6].label = std::string("SOUND");
	UINodes[6].type = UINode::UINodeType::UINodeType_Menu;
	UINodes[6].childrenIndices[0] = 12;
	UINodes[6].childrenIndices[1] = 13;
	UINodes[6].childrenIndices[2] = 14;
	UINodes[6].childrenCount = 3;

	UINodes[7].label = std::string("INPUT");
	UINodes[7].type = UINode::UINodeType::UINodeType_Menu;
	UINodes[7].childrenIndices[0] = 12;
	UINodes[7].childrenCount = 1;

	UINodes[8].label = std::string("KODKOD");
	UINodes[8].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[8].action = actionProgressDeveloper;

	UINodes[9].label = std::string("DEVELOPER");
	UINodes[9].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[9].action = actionEmpty;

	UINodes[10].label = std::string("ABOUT");
	UINodes[10].type = UINode::UINodeType::UINodeType_Menu;
	UINodes[10].childrenIndices[0] = 11;
	UINodes[10].childrenCount = 1;

	UINodes[11].label = std::string("...");
	UINodes[11].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[11].action = actionTellStory;

	UINodes[12].label = std::string("MASTER VOLUME");
	UINodes[12].type = UINode::UINodeType::UINodeType_IntPercentage;
	UINodes[12].minValue = 0;
	UINodes[12].maxValue = 100;
	UINodes[12].step = 10;
	UINodes[12].valuePtr = &rfct::userSettings::get().seriaizeData.masterVoicePercentage;

	UINodes[13].label = std::string("BACKGROUND SOUND");
	UINodes[13].type = UINode::UINodeType::UINodeType_IntPercentage;
	UINodes[13].minValue = 0;
	UINodes[13].maxValue = 100;
	UINodes[13].step = 10;
	UINodes[13].valuePtr = &rfct::userSettings::get().seriaizeData.backgroundVoicePercentage;

	UINodes[14].label = std::string("EFFECTS");
	UINodes[14].type = UINode::UINodeType::UINodeType_IntPercentage;
	UINodes[14].minValue = 0;
	UINodes[14].maxValue = 100;
	UINodes[14].step = 10;
	UINodes[14].valuePtr = &rfct::userSettings::get().seriaizeData.effectsVoicePercentage;
}
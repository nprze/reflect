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
		glm::vec3 color = {1.f, 1.f, 1.f};
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
	rfct::cleanupDecors();
}
void actionQuit(rfct::frameContext* ctx) {
	rfct::userSettings::get().dumpUserSettings();
	glfwSetWindowShouldClose(rfct::renderer::getRen().getWindow().GetHandle(), true);
}

rfct::gameState rfct::getState()
{
	if (input::getInput().openClosePauseMenu && timeSinceStateChange > 1.f) {
		timeSinceStateChange = 0.f;
		currentMenuElementSelectedIndex = 0;
		if (lastState == gameState::menu) {
			lastState = beforePauseMenuState;
			currentNodeIndex = -1;
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
	debugDraw::drawText("fps: " + std::to_string(fps), glm::vec2(0, 0), 0.08f);
	if (ctx->state != gameState::menu || currentNodeIndex == -1) return;

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
	float intensity = glm::sin(globalTime * 3.f) * 0.2f + .8f;
	float oneLineHeight = textScale * imageExtent.y;
	float totalHeight = UINodes[currentNodeIndex].childrenCount * oneLineHeight + (UINodes[currentNodeIndex].childrenCount - 1) * interline * imageExtent.y;
	float startY = 0.5f * imageExtent.y - (0.5f * totalHeight);
	font* defaultFont = renderer::getRen().getUIPipeline().getDefaultFont();

	for (uint32_t i = 0; i < UINodes[currentNodeIndex].childrenCount; i++) {
		uint32_t childIndex = UINodes[currentNodeIndex].childrenIndices[i];
		float width = defaultFont->getTextWidth(UINodes[childIndex].label, textScale * imageExtent.y);
		if (currentMenuElementSelectedIndex == i) {
			intensity = 1.f;
		}
		else {
			intensity = 0.5f;
		}
		float xPos = (0.5f) * imageExtent.x - 0.5f * width;
		float yPos = startY + i * (oneLineHeight + interline * imageExtent.y);
		// draw text
		renderer::getRen().getUIPipeline().addTextVerticesHeight(
			UINodes[childIndex].label,
			glm::vec2(
				xPos,
				yPos + (animationOffsetNorm * imageExtent.y)),
			textScale * imageExtent.y,
			UINodes[childIndex].color * intensity);
	}

	if ((input::getInput().selectMenu || input::getInput().leftRightMenu != 0) && changeSelectionCooldown <= 0.f) {
		changeSelectionCooldown = 0.25f;
		uint32_t selectedNode = UINodes[currentNodeIndex].childrenIndices[currentMenuElementSelectedIndex];
		switch (UINodes[selectedNode].type) {
			case UINode::UINodeType::UINodeType_Menu:{
				currentNodeIndex = selectedNode;
				currentMenuElementSelectedIndex = 0;
				break;
			}
			case UINode::UINodeType::UINodeType_ActionButton: {
				UINodes[selectedNode].action(ctx);
				break;
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
	UINodes[0].childrenCount = 3;

	UINodes[1].label = std::string("RESUME 0");
	UINodes[1].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[1].action = actionResume;

	UINodes[2].label = std::string("RESUME 1");
	UINodes[2].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[2].action = actionResume;

	UINodes[3].label = std::string("SAVE AND QUIT");
	UINodes[3].type = UINode::UINodeType::UINodeType_ActionButton;
	UINodes[3].action = actionQuit;
}

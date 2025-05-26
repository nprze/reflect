#pragma once
#include <glm/glm.hpp>
#include "context.h"
namespace rfct {
	class bindableImage;
	struct button {
		glm::vec2 min;
		glm::vec2 max;
		bindableImage* image;
		bool isClicked;
	};
	class input {
		
        vk::Extent2D* windowExtent;
		static input s_input;
		std::array<button, RFCT_MAX_BUTTON_COUNT> m_Buttons;
	public:
		std::array<button, RFCT_MAX_BUTTON_COUNT>& getButtons() { return m_Buttons; };
		inline static input& getInput() { return s_input; };
		input();
		void init();
		float jump;
		bool openMenu;

		float xAxis;
		float yAxis;
		float zAxis;
		float cameraXAxis;
		float cameraYAxis;
		float cameraZAxis;

		gameState m_previousState;
		float m_timeElapsedSinceStateChanged;

		void pollAndParseEvents(frameContext* context);
		button* addClickableButton(glm::vec2 pos, glm::vec2 size);
	};
}
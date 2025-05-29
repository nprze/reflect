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
        int pointerID = -1;
		inline void updateIsClicked(const glm::vec2& point, const int& action, const int& pointer) {
            if (pointerID == pointer || pointerID == -1) {
                if (point.x >= this->min.x && point.x <= this->max.x &&
                    point.y >= this->min.y && point.y <= this->max.y) {
                    if (action == 0) {
                        isClicked = true;
                        pointerID = pointer;
                    } else if (action == 1) {
                        isClicked = false;
                        pointerID = -1;
                    } else if (action == 2) {
                        isClicked = true;
                        pointerID = pointer;
                    }
                } else if (pointerID != -1 && action == 2) {
                    isClicked = false;
                    pointerID = -1;
                }
            }
		}
	
	};
	class input {
		
        vk::Extent2D* windowExtent;
		static input s_input;
	public:
		inline static input& getInput() { return s_input; };
		input();
		void init();
        void drawButtons();

		bool openMenu;

		float walk;
		float jump;

		float dashX;
		float dash45up;
		float dash45down;
		float dashY;


		gameState m_previousState;
		float m_timeElapsedSinceStateChanged;

		void pollAndParseEvents(frameContext* context);
		button* addClickableButton(glm::vec2 pos, glm::vec2 size);
	};
}
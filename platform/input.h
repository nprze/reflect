#pragma once
#include <glm/glm.hpp>
#include "context.h"
#include <unordered_set>

namespace rfct {
	class bindableImage;

    struct button {
        glm::vec2 min;
        glm::vec2 max;
        bindableImage* image;
        std::unordered_set<int> activePointers;  // Track all pointers pressing this button
        glm::vec2 lastTouchPos = { -1, -1 };

        inline void updateIsClicked(const glm::vec2& point, const int& action, const int& pointer) {
            // action: 0 = down, 1 = up, 2 = move (assuming these are the mappings)

            bool inside = (point.x >= this->min.x && point.x <= this->max.x &&
                           point.y >= this->min.y && point.y <= this->max.y);

            if (action == 0) {  // Pointer down
                if (inside) {
                    activePointers.insert(pointer);
                }
            } else if (action == 1) {  // Pointer up
                activePointers.erase(pointer);
            } else if (action == 2) {  // Pointer move
                // If pointer was pressing and now moved outside, remove it
                if (activePointers.count(pointer)) {
                    if (!inside) {
                        activePointers.erase(pointer);
                    }
                    // If still inside, do nothing, keep pressed
                }
                else if (inside) {
                    activePointers.insert(pointer);
                }
            }

            // Update isClicked based on whether any pointers are pressing
            isClicked = !activePointers.empty();
        }

        bool isClicked = false;  // Now reflects "pressed" state based on active pointers
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

		float dashDefault;
		float dashX;
		float dash45up;
		float dash45down;
		float dashY;

        bool hold;

		glm::vec2 dashHelper;

		gameState m_previousState;
		float m_timeElapsedSinceStateChanged;

		void pollAndParseEvents(frameContext* context);
		button* addClickableButton(glm::vec2 pos, glm::vec2 size);
	};
}
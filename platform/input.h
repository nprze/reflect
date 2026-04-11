#pragma once
#include <glm/glm.hpp>
#include <unordered_set>

namespace rfct {
	class bindableImage;
    struct button {
        glm::vec2 minViewport;
        glm::vec2 maxViewport;
        glm::vec2 minImageReleased;
        glm::vec2 maxImageReleased;
        std::unordered_set<int> activePointers;
        glm::vec2 lastTouchPos = { -1, -1 };
        bool isClicked = false;

        inline void updateIsClicked(const glm::vec2& point, const int& action, const int& pointer) {
            RFCT_PROFILE_FUNCTION();
            bool inside = (point.x >= this->minViewport.x && point.x <= this->maxViewport.x &&
                           point.y >= this->minViewport.y && point.y <= this->maxViewport.y);

            if (action == 0) { 
                if (inside) {
                    activePointers.insert(pointer);
                }
            } else if (action == 1) {
                activePointers.erase(pointer);
            } else if (action == 2) { 
                if (activePointers.count(pointer)) {
                    if (!inside) {
                        activePointers.erase(pointer);
                    }
                }
                else if (inside) {
                    activePointers.insert(pointer);
                }
            }

            isClicked = !activePointers.empty();
        }
    };
	class input {
	public:
        static input& getInput();
    public:
		input();
		void init();
        void drawButtons();
        void pollAndParseEvents(frameContext* context);
        button* addClickableButton(glm::vec2 pos, glm::vec2 size);

        // helper
        vk::Extent2D* windowExtent;
		glm::vec2 dashHelper;
        
        // gameplay inputs
		float walk;
		float jump;
		float dashDefault;
		float dashX;
		float dash45up;
		float dash45down;
		float dashY;
        float upDown;
        bool hold;

		// menu input
		bool openClosePauseMenu;
		bool selectMenu;
		float upDownMenu;
		float leftRightMenu;

        // dialogue input 
        bool anyClicked;
	};
}
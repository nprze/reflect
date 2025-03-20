#pragma once
#include <vector>
namespace rfct {
	class input {
        vk::Extent2D& windowExtent;
		static input* s_input;
	public:
		inline static input& getInput() { return *s_input; };
		inline static void setInput(input* in) { s_input = in; };
		input();
		float xAxis;
		float yAxis;
		float zAxis;
		float cameraXAxis;
		float cameraYAxis;
		float cameraZAxis;
		void pollEvents();
	};
}
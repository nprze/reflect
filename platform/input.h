#pragma once
namespace rfct {
	class input {
        vk::Extent2D* windowExtent;
		static input s_input;
	public:
		inline static input& getInput() { return s_input; };
		input();
		void init();
		float xAxis;
		float yAxis;
		float zAxis;
		float cameraXAxis;
		float cameraYAxis;
		float cameraZAxis;
		void pollEvents();
	};
}
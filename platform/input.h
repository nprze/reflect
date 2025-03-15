#pragma once
#include <vector>
namespace rfct {
	/*
	struct androidEvent {
		uint32_t action;
		float x;
		float y;
	};
	struct windowsClickEvent {
		uint32_t action;
		float x;
		float y;
	};
	struct windowsButtonEvent {
		uint32_t keyCode;
	};*/
	class input {
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
		void passEvents();//on android
	};
}
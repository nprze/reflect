#pragma once
#include <vector>
namespace rfct {
	class inputEvent {
		int key;
	};
	class inputLayer {
	public:
		virtual void updateInputs() = 0;
		virtual void updateBindings() = 0;
		virtual std::vector<inputEvent> getCurrentFrameEvents() = 0;
	};
}
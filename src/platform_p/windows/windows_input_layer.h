#pragma once
#include "platform_p/input_layer.h"
namespace rfct {
	class windowsInputLayer : public inputLayer {
	public:
		void updateInputs() override;
		void updateBindings() override;
		std::vector<inputEvent> getCurrentFrameEvents() override;
	};
}

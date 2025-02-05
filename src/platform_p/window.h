#pragma once
#include <vector>
#include "input_layer.h"
namespace rfct {
    class windowAbstact {
    public:
        /*
        windowAbstact(int width, int height, const char* title) { create(width, height, title); };
        ~windowAbstact() { destroy(); };
        */

        virtual void create(int width, int height, const char* title) = 0;
        virtual void destroy() = 0;
        virtual void show() = 0;
        virtual void hide() = 0;
		virtual vk::SurfaceKHR createSurface(vk::Instance instance) = 0;

        virtual bool pollEvents() = 0;

        virtual inputLayer* getInputLayer() = 0;
    };
}
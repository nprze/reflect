#pragma once
#include <vector>
namespace rfct {
    class windowAbstact {
    public:

        virtual void create(int width, int height, const char* title) = 0;
        virtual void destroy() = 0;
        virtual void show() = 0;
        virtual void hide() = 0;
		virtual vk::Extent2D getExtent() = 0;
        virtual void setExtent(vk::Extent2D ext) = 0;
		virtual vk::SurfaceKHR createSurface(vk::Instance instance) = 0;

        virtual bool pollEvents() = 0;

    };
}
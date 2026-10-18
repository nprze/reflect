#pragma once

namespace rfct {
    class RfctWindowAbstact {
    public:
        virtual vk::Extent2D GetExtent() = 0;
        virtual void SetExtent(vk::Extent2D ext) = 0;
        virtual void Create(const char* title, bool maximized = true, int width = 0, int height = 0) = 0;
        virtual void Destroy() = 0;
        virtual void Show() = 0;
        virtual void Hide() = 0;
		virtual vk::SurfaceKHR CreateSurface(vk::Instance instance) = 0;
        virtual bool PollAndParseEvents() = 0;
    };
}
#include "dot_Device.h"
#include "dot_Exception.h"

namespace dot
{
    Device::Device(Window& wnd)
        : wnd(wnd)
    {
        createSurface();
    }

    Device::~Device()
    {
        inst.getVkInstance().destroySurfaceKHR(surface);
    }

    void Device::createSurface()
    {
        VkSurfaceKHR vkSurface;

        if(glfwCreateWindowSurface(inst.getVkInstance(), wnd, nullptr, &vkSurface) != VK_SUCCESS)
            throw DOT_RUNTIME("Failed to create window surface!");

        surface = vk::SurfaceKHR(vkSurface);
    }
}
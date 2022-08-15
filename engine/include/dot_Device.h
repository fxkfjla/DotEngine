#pragma once

#include "dot_Vulkan.h"
#include "dot_Instance.h"

#include "Window.h"

namespace dot
{
    class Device
    {
    public:
        Device(Window&);
        Device(const Device&) = delete;
        Device(const Device&&) = delete;
        Device& operator=(const Device&) = delete;
        Device& operator=(const Device&&) = delete;
        ~Device();
    private:
        void createSurface();
        // void selectPhysicalDevice();
        // bool deviceSupported();
        // void setQueueFamilies();
        // bool deviceExtensionsSupported();

        vk::SurfaceKHR surface;
        vk::PhysicalDevice physicalDevice;
        vk::Device device;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        dot::Instance inst;
        Window& wnd;
    };
}
#pragma once

#include "dot_Vulkan.h"
#include "dot_Device.h"
#include "dot_Swapchain.h"

#include "Window.h"

class Graphics
{
public:
    Graphics(Window&);
    Graphics(const Graphics&) = delete;
    Graphics(const Graphics&&) = delete;
    Graphics& operator=(const Graphics&) = delete;
    Graphics& operator=(const Graphics&&) = delete;

    void beginFrame();
    void endFrame();
private:
    dot::Device device;
    dot::Swapchain swapchain;

    Window& wnd;
};
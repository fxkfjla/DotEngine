#pragma once

#include "dot_Vulkan.h"
#include "dot_Device.h"
#include "dot_Renderer.h"

#include "Window.h"

#include <memory>

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
    void drawFrame();
private:
    Window& wnd;
    dot::Device device;
    dot::Renderer renderer;
};
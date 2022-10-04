#pragma once

#include "dot_Device.h"
#include "dot_Renderer.h"
#include "dot_Model.h"

#include "Window.h"

#include <memory>

namespace dot
{
    class Engine
    {
    public:
        Engine(Window&);
        void run();
    private:
        void loadModels();
        void updateFrame();
        void renderFrame();

        Window& wnd;
        Device device;
        Renderer renderer;

        std::unique_ptr<Model> triangle;
    };
}
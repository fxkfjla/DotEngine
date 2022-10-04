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
        void updateFrame();
        void renderFrame();

        Window& wnd;
        Device device;
        Renderer renderer;

        std::vector<Model::Vertex> verticies =
        {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        std::unique_ptr<Model> model;
    };
}
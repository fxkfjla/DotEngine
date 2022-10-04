#include "dot_Engine.h"

namespace dot
{
    Engine::Engine(Window& wnd)
        : wnd(wnd), device(wnd), renderer(wnd, device)
    {
        loadModels();
    }

    void Engine::run()
    {
        while(!glfwWindowShouldClose(wnd))
        {
            glfwPollEvents();

            renderer.beginFrame();
            updateFrame();
            renderFrame();
            renderer.endFrame();
        }
    }

    void Engine::updateFrame()
    {

    }

    void Engine::renderFrame()
    {
        const auto& cmdBufferGfx = renderer.getCurrentCmdBufferGfx();

        triangle->bind(cmdBufferGfx);
        triangle->draw(cmdBufferGfx);
    }

    void Engine::loadModels()
    {
        std::vector<Model::Vertex> verticies =
        {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        triangle = std::make_unique<Model>(device, verticies);
    }
}
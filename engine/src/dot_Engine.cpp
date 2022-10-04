#include "dot_Engine.h"

namespace dot
{
    Engine::Engine(Window& wnd)
        : wnd(wnd), device(wnd), renderer(wnd, device)
    {
        model = std::make_unique<Model>(device, verticies);
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

        model->bind(cmdBufferGfx);
        model->draw(cmdBufferGfx);
    }
}
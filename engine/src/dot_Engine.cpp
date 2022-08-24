#include "dot_Engine.h"

namespace dot
{
    Engine::Engine(Window& wnd)
        : wnd(wnd), gfx(wnd)
    {

    }

    void Engine::run()
    {
        while(!glfwWindowShouldClose(wnd))
        {
            glfwPollEvents();

            gfx.beginFrame();
            gfx.drawFrame();
            gfx.endFrame();
        }
    }

    void Engine::updateFrame()
    {

    }

    void Engine::renderFrame()
    {

    }
}
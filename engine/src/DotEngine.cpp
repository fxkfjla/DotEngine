#include "DotEngine.h"

namespace Dot
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

            updateFrame();
            renderFrame();
        }
    }

    void Engine::updateFrame()
    {

    }

    void Engine::renderFrame()
    {

    }
}
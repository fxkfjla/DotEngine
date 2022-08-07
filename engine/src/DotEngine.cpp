#include "DotEngine.h"

DotEngine::DotEngine(Window& wnd)
    : wnd(wnd), gfx(wnd)
{

}

void DotEngine::run()
{
    while(!glfwWindowShouldClose(wnd))
    {
        glfwPollEvents();

        updateFrame();
        renderFrame();
    }
}

void DotEngine::updateFrame()
{

}

void DotEngine::renderFrame()
{

}
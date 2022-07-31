#include "App.h"

App::App(Window& wnd)
    : wnd(wnd), gfx(wnd)
{

}

void App::run()
{
    while(!glfwWindowShouldClose(wnd))
    {
        glfwPollEvents();

        updateFrame();
        renderFrame();
    }
}

void App::updateFrame()
{

}

void App::renderFrame()
{

}
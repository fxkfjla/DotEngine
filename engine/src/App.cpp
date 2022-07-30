#include "App.h"

App::App(Window& wnd)
    : wnd(wnd)
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
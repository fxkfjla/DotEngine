#include "App.h"

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
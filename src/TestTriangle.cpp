#include "TestTriangle.h"

void TestTriangle::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void TestTriangle::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // no opengl context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // no resize
    
    window = glfwCreateWindow(width, height, "DotEngine", nullptr, nullptr); // default monitor, opengl
}

void TestTriangle::initVulkan()
{
}

void TestTriangle::mainLoop()
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void TestTriangle::cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

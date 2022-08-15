#include "Window.h"

#include "dot_Exception.h"

Window::Window()
{
    if(!glfwInit()) // intializing glfw library
        throw DOT_RUNTIME("Failed to initalize GLFW library!");

    initGLFWhints();

    if(!(pWnd = glfwCreateWindow(width, height, "DotEngine", nullptr, nullptr)))
    {
        glfwTerminate();
        throw DOT_RUNTIME("Failed to create GLFW window!");
    }
}

Window::~Window()
{
    glfwDestroyWindow(pWnd);
    glfwTerminate();
}

Window::operator GLFWwindow *() const noexcept
{
    return pWnd;
}

void Window::initGLFWhints() const noexcept
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // no OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // window not resizable
}
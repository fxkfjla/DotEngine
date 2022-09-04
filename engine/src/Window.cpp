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

    glfwSetWindowUserPointer(pWnd, this);
    glfwSetFramebufferSizeCallback(pWnd, framebufferResizeCallback);
}  

Window::~Window()
{
    glfwDestroyWindow(pWnd);
    glfwTerminate();
}

bool Window::Resized() const noexcept
{
    return _resized;
}

void Window::Resized(bool state) noexcept
{
    _resized = state;
}

Window::operator GLFWwindow *() const noexcept
{
    return pWnd;
}

void Window::initGLFWhints() const noexcept
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // no OpenGL context
    glfwWindowHint(GLFW_RESIZABLE,  GLFW_TRUE);
}

void Window::framebufferResizeCallback(GLFWwindow* pWnd, int width, int height) noexcept
{
    glfwWaitEvents();
    auto wnd = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWnd));
    wnd->Resized(true);
    wnd->width = width;
    wnd->height = height;
}
#pragma once

#include <GLFW/glfw3.h>

class Window
{
public:
    Window();
    ~Window();
    bool Resized() const noexcept;
    void Resized(bool) noexcept;
    operator GLFWwindow*() const noexcept;
private:
    void initGLFWhints() const noexcept;
    static void framebufferResizeCallback(GLFWwindow*, int width, int height) noexcept;

    GLFWwindow* pWnd;
    int width = 800;
    int height = 600; 
    bool _resized = false;
};
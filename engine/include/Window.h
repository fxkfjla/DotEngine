#pragma once

#include <GLFW/glfw3.h>

class Window
{
public:
    Window();
    ~Window();
    operator GLFWwindow*() const noexcept;
private:
    void initGLFWhints() const noexcept;

    GLFWwindow* pWnd;
    static constexpr int width = 800;
    static constexpr int height = 600; 
};
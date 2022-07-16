#pragma once

#include "DotGLFW.h"

#include <iostream>

class Window
{
    // class WindowException : public std::exception
    // {
    // public:
    //     const char* what() const noexcept override;
    // };
public:
    Window();
    ~Window();
    operator GLFWwindow*() const noexcept;
private:
    void initGLFWhints() const noexcept;
private:
    GLFWwindow* pWnd;
    static constexpr int width = 800;
    static constexpr int height = 600; 
};
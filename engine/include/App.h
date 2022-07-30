#pragma once

#include "Window.h"
#include "Graphics.h"

class App
{
public:
    App(Window&);
    void run();
private:
    void updateFrame();
    void renderFrame();
private:
    Window& wnd;
     
    Graphics gfx;
};
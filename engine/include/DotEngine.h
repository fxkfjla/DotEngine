#pragma once

#include "Window.h"
#include "Graphics.h"

class DotEngine
{
public:
    DotEngine(Window&);
    void run();
private:
    void updateFrame();
    void renderFrame();

    Window& wnd;
    Graphics gfx;
};
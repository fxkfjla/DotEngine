#pragma once

#include "Window.h"
#include "Graphics.h"

namespace Dot
{
    class Engine
    {
    public:
        Engine(Window&);
        void run();
    private:
        void updateFrame();
        void renderFrame();

        Window& wnd;
        Graphics gfx;
    };
}
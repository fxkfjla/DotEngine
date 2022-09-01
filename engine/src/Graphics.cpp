#include "Graphics.h"

Graphics::Graphics(Window& wnd)
    : wnd(wnd), device(wnd), renderer(wnd, device)
{

}

void Graphics::beginFrame()
{
    renderer.beginFrame();

    if(renderer.frameStarted())
        renderer.beginRenderPass();
}

void Graphics::drawFrame()
{
    if(renderer.frameStarted())
        renderer.drawFrame();
}

void Graphics::endFrame()
{
    if(renderer.frameStarted())
    {
        renderer.endRenderPass();
        renderer.endFrame();
    }
}
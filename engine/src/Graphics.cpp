#include "Graphics.h"

Graphics::Graphics(Window& wnd)
    : wnd(wnd), device(wnd), renderer(wnd, device)
{

}

void Graphics::beginFrame()
{
    renderer.beginFrame();
    renderer.beginRenderPass();
}

void Graphics::drawFrame()
{
    renderer.drawFrame();
}

void Graphics::endFrame()
{
    renderer.endRenderPass();
    renderer.endFrame();
}
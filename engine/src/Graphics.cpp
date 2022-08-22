#include "Graphics.h"

Graphics::Graphics(Window& wnd)
    : wnd(wnd), device(wnd), swapchain(wnd, device) 
{
    dot::PipelineConfig pipelineConfig;
    dot::Pipeline::defaultConfig(pipelineConfig, swapchain.getRenderPass());

    pPipeline = std::make_unique<dot::Pipeline>(device, "engine/shaders/vert.spv", "engine/shaders/frag.spv", pipelineConfig);
}

void Graphics::beginFrame()
{

}

void Graphics::endFrame()
{

}
#include "dot_Renderer.h"
#include "dot_Exception.h"

namespace dot
{
    Renderer::Renderer(Window& wnd, Device& device)
        : wnd(wnd), device(device)
    {
        createSwapchain();
        createPipeline("engine/shaders/vert.spv", "engine/shaders/frag.spv");
        allocateCmdBuffersGfx();
    }

    Renderer::~Renderer()
    {
        device.getVkDevice().waitIdle();

        device.getVkDevice().freeCommandBuffers(device.getCmdPoolGfx(), cmdBuffersGfx);
    }

    void Renderer::createSwapchain()
    {
        pPrevSwapchain = std::move(pSwapchain);
        pSwapchain = std::make_unique<Swapchain>(wnd, device);
    }

    void Renderer::createPipeline(const std::string& vertPath, const std::string& fragPath)
    {
        pPrevPipeline = std::move(pPipeline);

        dot::PipelineConfig pipelineConfig;
        dot::Pipeline::defaultConfig(pipelineConfig, pSwapchain->getRenderPass()); 
        pPipeline = std::make_unique<Pipeline>(device, vertPath, fragPath, pipelineConfig);
    }
    
    void Renderer::allocateCmdBuffersGfx()
    {
        vk::CommandBufferAllocateInfo allocInfo
        (
            device.getCmdPoolGfx(),             // commandPool
            vk::CommandBufferLevel::ePrimary,   // level    
            pSwapchain->getMaxFramesInFlight()  // commandBufferCount
        );

        try
        {
            cmdBuffersGfx = device.getVkDevice().allocateCommandBuffers(allocInfo);
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }

    void Renderer::drawFrame()
    {
        const auto& cmdBufferGfx = getCurrentCmdBufferGfx();
        cmdBufferGfx.bindPipeline(vk::PipelineBindPoint::eGraphics, *pPipeline);
        vkCmdDraw(cmdBufferGfx, 3, 1, 0, 0);
    }

    void Renderer::beginFrame()
    {
        currentImageIndex = pSwapchain->acquireNextImage();
        frameStarted = true;

        const auto& cmdBufferGfx = getCurrentCmdBufferGfx();

        try
        {
            vk::CommandBufferBeginInfo beginInfo;
            cmdBufferGfx.begin(beginInfo);
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }

    void Renderer::endFrame()
    {
        const auto& cmdBufferGfx = getCurrentCmdBufferGfx();
        try
        {
            cmdBufferGfx.end();
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
        pSwapchain->submitCmdBuffer(cmdBufferGfx, currentImageIndex);

        frameStarted = false;        
        currentFrameInFlight = (currentFrameInFlight + 1) % pSwapchain->getMaxFramesInFlight();
    }

    void Renderer::beginRenderPass()
    {
        vk::Rect2D renderArea(vk::Offset2D(0, 0), pSwapchain->getExtent());
        vk::ClearValue clearValue;

        vk::RenderPassBeginInfo beginInfo
        (
            pSwapchain->getRenderPass(),                        // renderPass
            pSwapchain->getFramebuffer(currentImageIndex),      // framebuffer
            renderArea,                                         // renderArea
            clearValue                                          // clearValue 
        );

        auto cmdBufferGfx = getCurrentCmdBufferGfx();
        cmdBufferGfx.beginRenderPass(beginInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport
        (
            0.0f, 0.0f,                             // x, y
            (float)pSwapchain->getExtent().width,   // width
            (float)pSwapchain->getExtent().height,  // height
            0.0f, 1.0f                              // minDepth, maxDepth
        );

        cmdBufferGfx.setViewport(0, viewport);
        cmdBufferGfx.setScissor(0, renderArea);
    }

    void Renderer::endRenderPass()
    {
        auto cmdBufferGfx = getCurrentCmdBufferGfx();
        cmdBufferGfx.endRenderPass();
    }

    const vk::CommandBuffer& Renderer::getCurrentCmdBufferGfx()
    {
        return cmdBuffersGfx[currentFrameInFlight];
    }
}
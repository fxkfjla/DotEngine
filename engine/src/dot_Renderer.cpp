#include "dot_Renderer.h"
#include "dot_Exception.h"

#include <iostream>

namespace dot
{
    Renderer::Renderer(Window& wnd, Device& device)
        : wnd(wnd), device(device)
    {
        recreateSwapchain();
        createPipeline("engine/shaders/vert.spv", "engine/shaders/frag.spv");
        allocateCmdBuffersGfx();
    }

    Renderer::~Renderer()
    {
        device.getVkDevice().waitIdle();

        device.getVkDevice().freeCommandBuffers(device.getCmdPoolGfx(), cmdBuffersGfx);
    }

    void Renderer::recreateSwapchain() noexcept
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(wnd, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(wnd, &width, &height);
            glfwWaitEvents();
        }

        device.getVkDevice().waitIdle();

        pSwapchain = std::make_unique<Swapchain>(wnd, device, std::move(pSwapchain));
    }

    void Renderer::createPipeline(const std::string& vertPath, const std::string& fragPath)
    {
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

    void Renderer::beginFrame()
    {
        const vk::Result& result = pSwapchain->acquireNextImage(currentImageIndex);

        if(result == vk::Result::eErrorOutOfDateKHR)
        {
            glfwWaitEvents();
            recreateSwapchain();
            return;
        }
        else if(result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
            throw DOT_RUNTIME("Failed to acquire swapchain image!");

        _frameStarted = true;

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

        beginRenderPass();
    }

    void Renderer::beginRenderPass() const noexcept
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
        cmdBufferGfx.bindPipeline(vk::PipelineBindPoint::eGraphics, *pPipeline);
    }

    void Renderer::endRenderPass() const noexcept
    {
        const auto& cmdBufferGfx = getCurrentCmdBufferGfx();
        cmdBufferGfx.endRenderPass();
    }

    void Renderer::endFrame()
    {
        endRenderPass();

        const auto& cmdBufferGfx = getCurrentCmdBufferGfx();
        try
        {
            cmdBufferGfx.end();
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }

        const vk::Result& result = pSwapchain->submitCmdBuffer(cmdBufferGfx, currentImageIndex);

        if(result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || wnd.Resized())
        {
            glfwWaitEvents();
            recreateSwapchain();
            wnd.Resized(false);
            _frameStarted = false;        
            return;
        }
        else if(result != vk::Result::eSuccess)
            throw DOT_RUNTIME("Failed to present swapchain image!");

        _frameStarted = false;        
        currentFrameInFlight = (currentFrameInFlight + 1) % pSwapchain->getMaxFramesInFlight();
    }

    const vk::CommandBuffer& Renderer::getCurrentCmdBufferGfx() const noexcept
    {
        return cmdBuffersGfx[currentFrameInFlight];
    }

    bool Renderer::frameStarted() const noexcept
    {
        return _frameStarted;
    }
}
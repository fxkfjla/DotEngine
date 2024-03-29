#pragma once

#include "dot_Device.h"
#include "dot_Swapchain.h"
#include "dot_Pipeline.h"

#include "Window.h"

#include <string>
#include <memory>
#include <vector>

namespace dot
{
    class Renderer
    {
    public:
        Renderer(Window&, Device&);
        Renderer(const Renderer&) = delete;
        Renderer(const Renderer&&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer& operator=(const Renderer&&) = delete;
        ~Renderer();
        void beginFrame();
        void endFrame();
        const vk::CommandBuffer& getCurrentCmdBufferGfx() const noexcept;
        bool frameStarted() const noexcept;
    private:
        void beginRenderPass() const noexcept;
        void endRenderPass() const noexcept;
        void recreateSwapchain() noexcept;
        void createPipeline(const std::string& vertPath, const std::string& fragPath);
        void allocateCmdBuffersGfx();

        Window& wnd;
        Device& device;
        std::unique_ptr<Swapchain> pSwapchain = nullptr;
        std::unique_ptr<Pipeline> pPipeline = nullptr;
        std::vector<vk::CommandBuffer> cmdBuffersGfx;

        size_t currentFrameInFlight = 0;
        uint32_t currentImageIndex = 0;
        bool _frameStarted = false;
    };
}
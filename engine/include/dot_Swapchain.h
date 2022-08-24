#pragma once

#include "dot_Vulkan.h"
#include "dot_Device.h"

#include "Window.h"

namespace dot
{
    class Swapchain
    {
    public:
        Swapchain(Window&, Device&);
        Swapchain(const Swapchain&) = delete;
        Swapchain(const Swapchain&&) = delete;
        Swapchain& operator=(const Swapchain&) = delete;
        Swapchain& operator=(const Swapchain&&) = delete;
        ~Swapchain();
        uint32_t acquireNextImage() const;
        void submitCmdBuffer(const vk::CommandBuffer&, uint32_t index);
        operator const vk::SwapchainKHR&() const noexcept;
        const vk::Extent2D& getExtent() const noexcept;
        const vk::RenderPass& getRenderPass() const noexcept;
        const vk::Framebuffer& getFramebuffer(size_t) const noexcept;
        size_t getMaxFramesInFlight() const noexcept;
    private:
        void createSwapchain();
        void createImageViews();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();
        void destroySyncObjects();
        vk::Extent2D getExtent(const vk::SurfaceCapabilitiesKHR&) const noexcept;
        vk::SurfaceFormatKHR getSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>&) const noexcept; 
        vk::PresentModeKHR getPresentMode(const std::vector<vk::PresentModeKHR>&) const noexcept;

        Window& wnd;
        Device& device;
        vk::Extent2D extent;
        vk::SurfaceFormatKHR surfaceFormat;
        vk::PresentModeKHR presentMode;
        vk::Format imageFormat;
        vk::SwapchainKHR swapchain;
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
        vk::RenderPass renderPass;
        std::vector<vk::Framebuffer> framebuffers;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> imageInFlightFences;

        size_t maxFramesInFlight;
        size_t currentFrameInFlight = 0;
    };
}
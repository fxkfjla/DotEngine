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
        operator const vk::SwapchainKHR&() const noexcept;
        const vk::RenderPass& getRenderPass() const noexcept;
    private:
        void createSwapchain();
        void createImageViews();
        void createRenderPass();
        void createFramebuffers();
        vk::Extent2D getExtent(const vk::SurfaceCapabilitiesKHR&) const noexcept;
        vk::SurfaceFormatKHR getSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>&) const noexcept; 
        vk::PresentModeKHR getPresentMode(const std::vector<vk::PresentModeKHR>&) const noexcept;

        vk::Extent2D extent;
        vk::SurfaceFormatKHR surfaceFormat;
        vk::PresentModeKHR presentMode;
        vk::Format imageFormat;
        vk::SwapchainKHR swapchain;
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
        vk::RenderPass renderPass;
        std::vector<vk::Framebuffer> framebuffers;

        Device& device;
        Window& wnd;
    };
}
#pragma once

#include "DotVulkan.h"
#include "Window.h"

#include <vector>
#include <optional>

class Graphics
{
public:
    Graphics(Window&);
    ~Graphics();
    void beginFrame();
    void endFrame();
private:
    void initVulkan();
    std::vector<const char*> getRequiredExtensions() const noexcept;
    bool validationLayersSupported() const noexcept;
    void displayExtensionsInfo(std::vector<const char*>) const noexcept;

    void initDebugMessenger() noexcept;
    void setDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT&) const noexcept;

    void initSurface();

    void selectPhysicalDevice();
    bool deviceIsSupported(const vk::PhysicalDevice&) noexcept;
    void setQueueFamilies(const vk::PhysicalDevice&) noexcept;
    bool deviceExtensionsSupported(const vk::PhysicalDevice&) const noexcept;
    void setSwapChainDetails(const vk::PhysicalDevice&) noexcept;

    void initLogicalDevice();

    void initSwapChain();
    vk::Extent2D getSwapExtent() const noexcept;
    vk::SurfaceFormatKHR getSwapSurfaceFormat() const noexcept;
    vk::PresentModeKHR getSwapPresentMode() const noexcept;

    void initImageViews();

    void initRenderPass();

    void initPipeline();

    void initFramebuffers();

    void initCommandPool();
    void initCommandBuffer();
    void recordCommandBuffer(const vk::CommandBuffer&, uint32_t imageIndex);

    void initSyncObjects();
    void destroySyncObjects();

    vk::Instance vkInst;
    vk::SurfaceKHR surface;
    vk::PhysicalDevice physicalDevice = nullptr;  // destroyed with VkInstance
    vk::Device device;
    vk::Queue graphicQueue;
    vk::Queue presentQueue;
    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::ImageView> swapChainImageViews;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    vk::RenderPass renderPass;
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline pipeline;

    std::vector<vk::Framebuffer> swapChainFramebuffers;
    // vk::

    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;

    Window& wnd;

    vk::Semaphore imageAvailableSemaphore;
    vk::Semaphore renderFinishedSemaphore;
    vk::Fence inFlightFence;

    const std::vector<const char*> deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicFamily;
        std::optional<uint32_t> presentFamily;

        bool found() const noexcept
        {
            return
                graphicFamily.has_value() &&
                presentFamily.has_value();
        }
    } queueIndices;
    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> modes;
    } swapChainDetails;

    vk::DispatchLoaderDynamic dldi;
    vk::DebugUtilsMessengerEXT debugMessenger;
    const std::vector<const char*> validationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };
    #ifdef NDEBUG
        const bool validationLayersEnabled = false;
    #else
        const bool validationLayersEnabled = true;
    #endif
};

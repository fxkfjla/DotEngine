#pragma once

#include "DotGLFW.h"
#include "Window.h"

#include <vector>
#include <optional>

class Graphics
{
public:
    Graphics(Window&);
    ~Graphics();
private:
    void initVulkan();
    std::vector<const char*> getRequiredExtensions() const noexcept;
    bool validationLayersSupported() const noexcept;

    void initDebugMessenger();
    void setDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&) noexcept;
    VkResult createDebugMessenger
    (
        VkInstance,
        const VkDebugUtilsMessengerCreateInfoEXT*,
        const VkAllocationCallbacks*,
        VkDebugUtilsMessengerEXT*
    ) noexcept;
    void destroyDebugMessenger
    (
        VkInstance,
        VkDebugUtilsMessengerEXT,
        const VkAllocationCallbacks*
    ) noexcept;

    void initSurface();

    void selectPhysicalDevice();
    bool deviceIsSupported(const VkPhysicalDevice&) noexcept;
    void setQueueFamilies(const VkPhysicalDevice&) noexcept;
    bool deviceExtensionsSupported(const VkPhysicalDevice&) const noexcept;
    void setSwapChainDetails(const VkPhysicalDevice&) noexcept;

    void initLogicalDevice();

    void initSwapChain();
    VkExtent2D getSwapExtent() const noexcept;
    VkSurfaceFormatKHR getSwapSurfaceFormat() const noexcept;
    VkPresentModeKHR getSwapPresentMode() const noexcept;

    VkInstance vkInst;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;  // destroyed with VkInstance
    VkDevice device;
    VkQueue graphicQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;

    Window& wnd;

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
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> modes;
    } swapChainDetails;

    VkDebugUtilsMessengerEXT debugMessenger;
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

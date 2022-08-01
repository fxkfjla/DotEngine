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
    void initDebugMessenger();
    void initSurface();
    void selectPhysicalDevice();
    void initLogicalDevice();
    std::vector<const char*> getRequiredExtensions() const noexcept;
    bool deviceIsSupported(const VkPhysicalDevice&) noexcept;
    bool deviceExtensionsSupported(const VkPhysicalDevice&) const noexcept;
// searching for device supported queues, almost every operation in vulkan is submitted to a queue (drawing, uploading textures etc)
    void setQueueFamiliesOf(const VkPhysicalDevice& device) noexcept;
    bool validationLayersSupported() const noexcept;
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
    void setDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&) noexcept;
private:
    VkInstance vkInst;
    VkDebugUtilsMessengerEXT debugMessenger;
    Window& wnd;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;  // destroyed with VkInstance
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
    VkDevice device;
    VkQueue graphicQueue;
    VkQueue presentQueue;
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

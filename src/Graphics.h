#pragma once

#include "DotGLFW.h"

#include <vector>

class Graphics
{
public:
    Graphics();
    ~Graphics();
private:
    void initVulkan();
    std::vector<const char*> getRequiredExtensions() noexcept;

    bool validationLayersSupported() noexcept;
    void initDebugMessenger();
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
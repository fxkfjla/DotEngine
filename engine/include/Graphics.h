#pragma once

#include "DotGLFW.h"

#include <vector>
#include <optional>

class Graphics
{
public:
    Graphics();
    ~Graphics();
private:
    void initVulkan();
    void initDebugMessenger();
    void selectPhysicalDevice();
    void initLogicalDevice();
    std::vector<const char*> getRequiredExtensions() const noexcept;
    bool deviceIsSupported(const VkPhysicalDevice&) noexcept;
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
    VkPhysicalDevice physicalDevice;  // destroyed with VkInstance
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicFamily;

        bool found() const noexcept
        {
            return
                graphicFamily.has_value();
        }
    } queueIndices;
    VkDevice device;
    VkQueue graphicQueue;
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

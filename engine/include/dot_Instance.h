#pragma once

#include "dot_Vulkan.h"

#include <string>
#include <vector>

namespace dot
{
    class Instance
    {
    public:
        Instance();
        ~Instance();
    private:
        std::vector<const char*> getRequiredExtensions() const noexcept;
        bool validationLayersSupported() const noexcept;
        void displayExtensionsInfo(std::vector<const char*>) const noexcept;
        void createDebugMessenger();
        void createDebugMessengerCreateInfo() noexcept;

        std::string appName = "DotEngine";
        vk::Instance inst;
        vk::DispatchLoaderDynamic dldi;

        #ifdef NDEBUG
            const bool validationLayersEnabled = false;
        #else
            const bool validationLayersEnabled = true;
        #endif
        vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
        vk::DebugUtilsMessengerEXT debugMessenger;
        const std::vector<const char*> validationLayers =
        {
            "VK_LAYER_KHRONOS_validation"
        };
    };
}
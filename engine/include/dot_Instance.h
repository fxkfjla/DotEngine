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
        Instance(const Instance&) = delete;
        Instance(const Instance&&) = delete;
        Instance& operator=(const Instance&) = delete;
        Instance& operator=(const Instance&&) = delete;
        ~Instance();
        operator const vk::Instance&() const noexcept;
        const vk::Instance& getVkInstance() const noexcept;
        bool validationLayersEnabled() const noexcept;
        const std::vector<const char*>& getValidationLayers() const noexcept;
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
            const bool _validationLayersEnabled = false;
        #else
            const bool _validationLayersEnabled = true;
        #endif
        vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
        vk::DebugUtilsMessengerEXT debugMessenger;
        const std::vector<const char*> validationLayers =
        {
            "VK_LAYER_KHRONOS_validation"
        };
    };
}
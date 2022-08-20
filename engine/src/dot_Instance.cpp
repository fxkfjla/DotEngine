#include "dot_Instance.h"
#include "dot_Exception.h"

#include "GLFW/glfw3.h"

#include <set>
#include <iostream>

namespace dot
{
    Instance::Instance()
    {
        vk::ApplicationInfo appInfo(
            appName.c_str(),            // pApplicationName
            1,                          // applicationVersion
            nullptr,                    // pEngineName
            1,                          // engineVersion
            VK_API_VERSION_1_1          // apiVersion
        );

        vk::InstanceCreateInfo createInfo
        (
            vk::InstanceCreateFlags(0U),
            &appInfo
        );

        const auto&& requiredExtensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        if(_validationLayersEnabled)
        {
            createDebugMessengerCreateInfo();

            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            createInfo.pNext = reinterpret_cast<vk::DebugUtilsMessengerCreateInfoEXT*>(&debugMessengerInfo); 
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        try
        {
            inst = vk::createInstance(createInfo);
            dldi = vk::DispatchLoaderDynamic(inst, vkGetInstanceProcAddr);
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }

        if(_validationLayersEnabled)
        {
            createDebugMessenger();
            displayExtensionsInfo(requiredExtensions);
        }
    }

    Instance::~Instance()
    {
        if(_validationLayersEnabled)
            inst.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);

        inst.destroy();
    }

    Instance::operator const vk::Instance&() const noexcept
    {
        return inst;
    }

    const vk::Instance& Instance::getVkInstance() const noexcept
    {
        return inst;
    }

    bool Instance::validationLayersEnabled() const noexcept
    {
        return _validationLayersEnabled;
    }

    const std::vector<const char*>& Instance::getValidationLayers() const noexcept
    {
        return validationLayers;
    }

    std::vector<const char*> Instance::getRequiredExtensions() const noexcept
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if(_validationLayersEnabled)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    bool Instance::validationLayersSupported() const noexcept
    {
        std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

        std::set<std::string> requiredLayers(validationLayers.begin(), validationLayers.end());

        for(const auto& layer : availableLayers)
            requiredLayers.erase(layer.layerName);

        return requiredLayers.empty();
    }

    void Instance::displayExtensionsInfo(std::vector<const char*> extensions) const noexcept
    {
        std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

        std::cout << "\nViable extensions:\n";
        for(const auto& supportedExtension : supportedExtensions)
            std::cout << '\t' << supportedExtension.extensionName << '\n';
        
        std::cout << "\nSupported extensions:\n";
        for(const auto& extension : extensions)
        {
            bool found = false;
            for(const auto& supportedExtension : supportedExtensions)
                if(strcmp(extension, supportedExtension.extensionName) == 0)
                {
                    std::cout << '\t' << extension << " is supported!\n";
                    found = true;
                    break;
                }

            if(!found)
                std::cout << '\t' << extension << " is not supported!\n";
        }

        std::cout << "\n";
    }

    void Instance::createDebugMessenger()
    {
        try
        {
            debugMessenger = inst.createDebugUtilsMessengerEXT(debugMessengerInfo, nullptr, dldi);
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback
    (
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    )
    {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << '\n';

        return VK_FALSE;
    }

    void Instance::createDebugMessengerCreateInfo() noexcept
    {
        debugMessengerInfo = vk::DebugUtilsMessengerCreateInfoEXT
        (
            vk::DebugUtilsMessengerCreateFlagsEXT(0U),                                                                  // flags
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |   // messageSeverity
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |        // messageType 
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            debugCallback                                                                                               // pFnUserCallback
        );
    }
}
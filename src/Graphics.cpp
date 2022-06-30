#include "Graphics.h"

#include <iostream>
#include <cstring>

Graphics::Graphics()
{
    initVulkan();
}

Graphics::~Graphics()
{
    vkDestroyInstance(vkInst, nullptr);
}

void Graphics::initVulkan()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "DotEngine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;

// vulkan is platform agnostic, meaning that we need window extension to interface with window system
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

// non-essential extensions
    // uint32_t optionalExtensionCount = 0;
    // vkEnumerateInstanceExtensionProperties(nullptr, &optionalExtensionCount, nullptr); // how many extensions are available

    // std::vector<VkExtensionProperties> optionalExtensions(optionalExtensionCount);
    // vkEnumerateInstanceExtensionProperties(nullptr, &optionalExtensionCount, optionalExtensions.data());

    instInfo.enabledExtensionCount = glfwExtensionCount;
    instInfo.ppEnabledExtensionNames = glfwExtensions;

    if(validationLayersEnabled && !validationLayersSupported())
        throw std::runtime_error("Requested validation layers are not supported!");

    if(validationLayersEnabled)
    {
        instInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
        instInfo.enabledLayerCount = 0;

    if(vkCreateInstance(&instInfo, nullptr, &vkInst) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the vk instance!");
    
    #ifndef NDEBUG
        std::cout<<"chuj";
    #endif
}

bool Graphics::validationLayersSupported() noexcept
{
    uint32_t validationLayersCount = 0;
    vkEnumerateInstanceLayerProperties(&validationLayersCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(validationLayersCount);
    vkEnumerateInstanceLayerProperties(&validationLayersCount, availableLayers.data());

    for(const char* layerName : validationLayers)
    {
        bool layerFound = false;
        for(const auto& availableLayer : availableLayers)
        {
            if(strcmp(layerName, availableLayer.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if(!layerFound)
            return false;
    }

    return true;
}
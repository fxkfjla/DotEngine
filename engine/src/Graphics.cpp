#include "Graphics.h"

#include <iostream>
#include <cstring>

Graphics::Graphics()
{
    initVulkan();
    initDebugMessenger();
}

Graphics::~Graphics()
{
    destroyDebugMessenger(vkInst, debugMessenger, nullptr);
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
    const auto&& requiredExtensions = getRequiredExtensions();
    instInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    instInfo.ppEnabledExtensionNames = requiredExtensions.data();

    if(validationLayersEnabled && !validationLayersSupported())
        throw std::runtime_error("Requested validation layers are not supported!");

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo; // outside scope to ensure it doesn't get destroyed before vkCreateInstance
    if(validationLayersEnabled)
    {
        instInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instInfo.ppEnabledLayerNames = validationLayers.data();
    // to be able to debug vkCreateInstance and vkDestroyInstance calls
        setDebugMessengerCreateInfo(debugMessengerInfo);
        instInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugMessengerInfo); 
    }
    else
    {
        instInfo.enabledLayerCount = 0;
        instInfo.pNext = nullptr;
    }

    if(vkCreateInstance(&instInfo, nullptr, &vkInst) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the vk instance!");
}

std::vector<const char*> Graphics::getRequiredExtensions() noexcept
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(validationLayersEnabled)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
// change to message box
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Graphics::initDebugMessenger()
{
    if(!validationLayersEnabled)
        return;
    
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo;
    setDebugMessengerCreateInfo(debugMessengerInfo);

    if(createDebugMessenger(vkInst, &debugMessengerInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        throw std::runtime_error("Failed to create a debug messenger!");
}

VkResult Graphics::createDebugMessenger
(
    VkInstance vkInst,
    const VkDebugUtilsMessengerCreateInfoEXT* pDebugMessengerInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
) noexcept
{
    auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInst, "vkCreateDebugUtilsMessengerEXT");
// because this function is an extension function it is not loaded, and we need to look it up ourselves
    if(fn != nullptr)
        return fn(vkInst, pDebugMessengerInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void Graphics::destroyDebugMessenger
(
    VkInstance vkInst,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) noexcept
{
    if(validationLayersEnabled)
    {
        auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInst, "vkDestroyDebugUtilsMessengerEXT");

        if(fn != nullptr)
            fn(vkInst, debugMessenger, pAllocator);
    }
}

void Graphics::setDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugMessengerInfo) noexcept
{
    debugMessengerInfo = {};
    debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    debugMessengerInfo.pfnUserCallback = debugCallback;
    debugMessengerInfo.pUserData = nullptr;
}

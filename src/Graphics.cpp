#include "Graphics.h"

#include <iostream>

Graphics::Graphics()
{

}

Graphics::~Graphics()
{

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

    instInfo.enabledExtensionCount = glfwExtensionCount;
    instInfo.ppEnabledExtensionNames = glfwExtensions;
    instInfo.enabledLayerCount = 0;
    // instInfo.ppEnabledLayerNames = nullptr;

    if(vkCreateInstance(&instInfo, nullptr, &vkInst) != VK_SUCCESS)
        throw std::runtime_error("Failed to create the vk instance!");
}
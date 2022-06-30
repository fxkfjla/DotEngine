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
private:
    VkInstance vkInst;

    const std::vector<const char*> validationLayers =
    {
        "VK_LAYER_KHRONOS_validation"
    };
    bool validationLayersSupported() noexcept;
    #ifdef NDEBUG
        const bool validationLayersEnabled = false;
    #else
        const bool validationLayersEnabled = true;
    #endif
};
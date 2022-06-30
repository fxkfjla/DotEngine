#pragma once

#include "DotGLFW.h"

class Graphics
{
public:
    Graphics();
    ~Graphics();
private:
    void initVulkan();
private:
    VkInstance vkInst;
};
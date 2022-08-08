#pragma once

#include "DotVulkan.h"

#include <string>
#include <fstream>
#include <vector>

class Shader
{
public:
    Shader(const std::string& filename, const vk::Device& device);
    ~Shader();
    void create();
    const vk::ShaderModule& getModule() const noexcept;
private:
    std::vector<char> readFile() const;
    vk::ShaderModule createShaderModule() const;

    std::string filename;
    std::vector<char> data;
    vk::ShaderModule shaderModule;

    const vk::Device& device;
};
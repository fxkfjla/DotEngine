#pragma once

#include "dot_Vulkan.h"

#include <string>
#include <fstream>
#include <vector>

class Shader
{
public:
    Shader(const vk::Device&);
    Shader(const vk::Device&, const std::string& filename);
    ~Shader();
    operator const vk::ShaderModule&() const noexcept;
    void read(const std::string& filename);
    const vk::ShaderModule& getModule() const noexcept;
private:
    std::vector<char> readFile() const;
    vk::ShaderModule createShaderModule() const;

    std::string filename;
    std::vector<char> data;
    vk::ShaderModule shaderModule;

    const vk::Device& device;
};
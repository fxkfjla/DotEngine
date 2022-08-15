#include "Shader.h"
#include "dot_Exception.h"

#include <iostream>

Shader::Shader(const std::string& filename, const vk::Device& device)
    : filename(filename), device(device)
{
    try
    {
        data = readFile();
        shaderModule = createShaderModule();
    }
    catch(const std::runtime_error& e)
    {
        throw e;
    }
}

Shader::~Shader()
{
    device.destroyShaderModule(shaderModule);
}

std::vector<char> Shader::readFile() const
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if(!file)
        throw std::runtime_error("Failed to open the file!");

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

vk::ShaderModule Shader::createShaderModule() const
{
    vk::ShaderModuleCreateInfo createInfo = {};
    createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
    createInfo.codeSize = data.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

    try
    {
        vk::ShaderModule shaderModule = device.createShaderModule(createInfo);
        return shaderModule;
    }
    catch(const std::runtime_error& e)
    {
        throw e; 
    }
}

const vk::ShaderModule& Shader::getModule() const noexcept
{
    return shaderModule;
}
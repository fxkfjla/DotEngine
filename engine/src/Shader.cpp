#include "Shader.h"
#include "dot_Exception.h"

Shader::Shader(const vk::Device& device)
    : device(device){}

Shader::Shader(const vk::Device& device, const std::string& filename)
    : device(device), filename(filename)
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

Shader::operator const vk::ShaderModule&() const noexcept
{
    return shaderModule;
}

void Shader::read(const std::string& filename)
{
    this->filename = filename;

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
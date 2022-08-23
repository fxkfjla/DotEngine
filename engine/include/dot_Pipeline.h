#pragma once

#include "dot_Device.h"
#include "dot_Swapchain.h"

#include "Shader.h"

#include <vector>
#include <string>

namespace dot
{
    struct PipelineConfig
    {
        PipelineConfig() = default;
        PipelineConfig(const PipelineConfig&) = delete;
        PipelineConfig(const PipelineConfig&&) = delete;
        PipelineConfig& operator=(const PipelineConfig&) = delete;
        PipelineConfig& operator=(const PipelineConfig&&) = delete;

        vk::PipelineVertexInputStateCreateInfo vertexStateInfo;           //
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo;  // determines topology: triangles, lines, points
        vk::PipelineTessellationStateCreateInfo tesselationStateInfo;     //
        vk::PipelineViewportStateCreateInfo viewportStateInfo;            //
        vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo;  //
        vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;      //
        vk::PipelineDepthStencilStateCreateInfo stencilStateInfo;         //
        vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;  //
        vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo;        //
        std::vector<vk::DynamicState> dynamicStates;                      // 
        vk::PipelineDynamicStateCreateInfo dynamicStateInfo;              // allows some changes to pipeline without need to rebuild               
        vk::PipelineLayoutCreateInfo layoutInfo;                          // information about push-constants and descriptors (ways to provide data to shaders)
        vk::RenderPass renderPass;                                        //
        uint32_t subpass;                                                 //
    };

    class Pipeline
    {
    public:

        Pipeline(Device&, const std::string& vertShaderPath, const std::string& fragShaderPath, const PipelineConfig&);
        Pipeline(const Pipeline&) = delete;
        Pipeline(const Pipeline&&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&&) = delete;
        operator const vk::Pipeline&() const noexcept;
        ~Pipeline();

        static void defaultConfig(PipelineConfig&, const vk::RenderPass&);
    private:
        void createLayout(const PipelineConfig&);
        void createPipeline(const PipelineConfig&);

        Shader vertShader;
        Shader fragShader;
        vk::PipelineLayout layout;
        vk::Pipeline pipeline;

        Device& device;
    };
}
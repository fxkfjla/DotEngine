#include "dot_Pipeline.h"
#include "dot_Model.h"
#include "dot_Exception.h"

#include "Shader.h"

namespace dot
{
    Pipeline::Pipeline
    (
        Device& device, 
        const std::string& vertPath, const std::string& fragPath, 
        const PipelineConfig& pipelineConfig
    )
    : device(device), vertShader(device), fragShader(device)
    {
        try
        {
            vertShader.read("engine/shaders/vert.spv");
            fragShader.read("engine/shaders/frag.spv");
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }

        createLayout(pipelineConfig);
        createPipeline(pipelineConfig);
    }

    Pipeline::~Pipeline()
    {
        device.getVkDevice().destroyPipeline(pipeline);
        device.getVkDevice().destroyPipelineLayout(layout);
    }

    Pipeline::operator const vk::Pipeline&() const noexcept
    {
        return pipeline;
    }

    void Pipeline::createLayout(const PipelineConfig& pipelineConfig)
    {
        try
        {
            layout = device.getVkDevice().createPipelineLayout(pipelineConfig.layoutInfo);
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }

    void Pipeline::createPipeline(const PipelineConfig& pipelineConfig)
    {
        vk::PipelineShaderStageCreateInfo vertShaderStageInfo
        (
            vk::PipelineShaderStageCreateFlags(0U), // flags
            vk::ShaderStageFlagBits::eVertex,       // stage
            vertShader,                             // module
            "main"                                  // pName
        );

        vk::PipelineShaderStageCreateInfo fragShaderStageInfo
        (
            vk::PipelineShaderStageCreateFlags(0U), // flags
            vk::ShaderStageFlagBits::eFragment,     // stage
            fragShader,                             // module
            "main"                                  // pName
        );

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesInfo = {vertShaderStageInfo, fragShaderStageInfo};

        vk::GraphicsPipelineCreateInfo createInfo
        (
            vk::PipelineCreateFlags(0U),            // flags
            shaderStagesInfo,                       // shaderStagesInfo
            &pipelineConfig.vertexStateInfo,        // pVertexInputState
            &pipelineConfig.inputAssemblyStateInfo, // pInputAssemblyState
            &pipelineConfig.tessellationStateInfo,  // pTessellationState
            &pipelineConfig.viewportStateInfo,      // pViewportState
            &pipelineConfig.rasterizationStateInfo, // pRasterizationState
            &pipelineConfig.multisampleStateInfo,   // pMultisampleState
            &pipelineConfig.stencilStateInfo,       // pDepthStencilState
            &pipelineConfig.colorBlendStateInfo,    // pColorBlendState
            &pipelineConfig.dynamicStateInfo,       // pDynamicState
            layout,                                 // layout
            pipelineConfig.renderPass,              // renderPass
            pipelineConfig.subpass                  // subpass 
        );

        try
        {
            pipeline = device.getVkDevice().createGraphicsPipeline(nullptr, createInfo).value;
        }
        catch(const std::runtime_error& e)
        {
            throw DOT_RUNTIME_WHAT(e);
        }
    }

    void Pipeline::defaultConfig(PipelineConfig& pipelineConfig, const vk::RenderPass& renderPass)
    {
        pipelineConfig.bindingDescriptions = std::move(Model::Vertex::getBindingDescription());
        pipelineConfig.attributeDescriptions = std::move(Model::Vertex::getAttributeDescription());

        pipelineConfig.vertexStateInfo = vk::PipelineVertexInputStateCreateInfo
        (
            vk::PipelineVertexInputStateCreateFlags(0U),    // flags
            pipelineConfig.bindingDescriptions,             // vertexBindingDescriptions
            pipelineConfig.attributeDescriptions            // vertexAttributeDescriptions
        );

        pipelineConfig.inputAssemblyStateInfo = vk::PipelineInputAssemblyStateCreateInfo
        (
            vk::PipelineInputAssemblyStateCreateFlags(0U),  // flags
            vk::PrimitiveTopology::eTriangleList            // topology
        );

        pipelineConfig.viewportStateInfo = vk::PipelineViewportStateCreateInfo
        (
            vk::PipelineViewportStateCreateFlags(0U),   // flags
            1,                                          // viewportCount
            nullptr,                                    // pViewports
            1,                                          // scissorCount
            nullptr                                     // pScissors
        );

        pipelineConfig.rasterizationStateInfo = vk::PipelineRasterizationStateCreateInfo
        (
            vk::PipelineRasterizationStateCreateFlags(0U),  // flags
            VK_FALSE,                                       // depthClampEnable
            VK_FALSE,                                       // rasterizerDiscardEnable
            vk::PolygonMode::eFill,                         // polygonMode
            vk::CullModeFlagBits::eBack,                    // cullMode
            vk::FrontFace::eClockwise,                      // frontFace
            VK_FALSE,                                       // depthBiasEnable
            0.0f,                                           // depthBiasConstantFactor
            0.0f,                                           // depthBiasClamp
            0.0f,                                           // depthBiasSlopeFactor
            1.0f                                            // lineWidth
        );

        pipelineConfig.multisampleStateInfo = vk::PipelineMultisampleStateCreateInfo
        (
            vk::PipelineMultisampleStateCreateFlags(0U),    // flags
            vk::SampleCountFlagBits::e1,                    // rasterizationSamples
            VK_FALSE,                                       // sampleShadingEnable
            1.0f                                            // minSampleShading
        );

        pipelineConfig.colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState
        (
            VK_FALSE,                                                           // blendEnable
            vk::BlendFactor::eOne,                                              // srcColorBlendFactor
            vk::BlendFactor::eZero,                                             // dstColorBlendFactor
            vk::BlendOp::eAdd,                                                  // colorBlendOp
            vk::BlendFactor::eOne,                                              // srcAlphaBlendFactor
            vk::BlendFactor::eZero,                                             // dstAlphaBlendFactor
            vk::BlendOp::eAdd,                                                  // alphaBlendOp
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |   // colorWriteMask   
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA     
        );

        pipelineConfig.colorBlendStateInfo = vk::PipelineColorBlendStateCreateInfo
        (
            vk::PipelineColorBlendStateCreateFlags(0U), // flags
            VK_FALSE,                                   // logicOpEnable
            vk::LogicOp::eCopy,                         // logicOp
            pipelineConfig.colorBlendAttachmentState,   // attachments
            {0.0f, 0.0f, 0.0f, 0.0f}                    // blendConstants
        );

        pipelineConfig.dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        pipelineConfig.dynamicStateInfo = vk::PipelineDynamicStateCreateInfo
        (
            vk::PipelineDynamicStateCreateFlags(0U),    // flags
            pipelineConfig.dynamicStates                // dynamicStates
        );

        pipelineConfig.renderPass = renderPass;

        pipelineConfig.subpass = 0;
    }
}
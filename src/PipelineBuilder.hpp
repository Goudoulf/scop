#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>

class PipelineBuilder {
   public:
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    vk::PipelineRasterizationStateCreateInfo rasterizer;
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    vk::PipelineColorBlendStateCreateInfo colorBlending;
    vk::PipelineMultisampleStateCreateInfo multisampling;
    vk::PipelineDepthStencilStateCreateInfo depthStencil;
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

    vk::PipelineRenderingCreateInfo renderInfo;

    PipelineBuilder() { clear(); }

    void clear();

    vk::raii::Pipeline build(const vk::raii::Device& device,
                             const vk::raii::PipelineLayout& layout);
};

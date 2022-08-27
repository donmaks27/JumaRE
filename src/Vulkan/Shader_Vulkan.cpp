// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN)

#include "Shader_Vulkan.h"

#include <fstream>

#include "RenderEngine_Vulkan.h"
#include "../../include/JumaRE/material/ShaderUniformInfo.h"

namespace JumaRenderEngine
{
    bool CreateVulkanShaderModule(VkShaderModule& outShaderModule, VkDevice device, const jstring& fileName, const bool optional)
    {
        std::ifstream file(*fileName, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            if (!optional)
            {
                JUTILS_LOG(error, JSTR("Can't open file {}"), fileName);
                return false;
            }
            outShaderModule = nullptr;
            return true;
        }

        jarray<char> data(static_cast<int32>(file.tellg()), 0);
        if (!data.isEmpty())
        {
            file.seekg(0, std::ios::beg);
            file.read(data.getData(), data.getSize());
        }
        file.close();
        if (data.isEmpty())
        {
            JUTILS_LOG(error, JSTR("Empty shader file {}"), fileName);
            return false;
        }

        VkShaderModuleCreateInfo shaderInfo{};
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.codeSize = data.getSize();
        shaderInfo.pCode = reinterpret_cast<const uint32*>(data.getData());
        const VkResult result = vkCreateShaderModule(device, &shaderInfo, nullptr, &outShaderModule);
        if (result != VK_SUCCESS)
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create shader module {}"), fileName);
            return false;
        }
        return true;
    }
    bool CreateVulkanShaderModule(VkShaderModule& outShaderModule, VkDevice device, const jmap<ShaderStageFlags, jstring>& fileNames, 
        const ShaderStageFlags shaderStage, const jstring& fileNamePostfix, const bool optional)
    {
        const jstring* fileName = fileNames.find(shaderStage);
        if (fileName == nullptr)
        {
            if (!optional)
            {
                JUTILS_LOG(error, JSTR("Missed file for required shader stage"));
                return false;
            }
            outShaderModule = nullptr;
            return true;
        }
        return CreateVulkanShaderModule(outShaderModule, device, *fileName + fileNamePostfix, optional);
    }

    Shader_Vulkan::~Shader_Vulkan()
    {
        clearVulkan();
    }

    bool Shader_Vulkan::initInternal(const jmap<ShaderStageFlags, jstring>& fileNames)
    {
        VkDevice device = getRenderEngine<RenderEngine_Vulkan>()->getDevice();
        if (!createShaderModules(device, fileNames))
        {
            JUTILS_LOG(error, JSTR("Failed to create vulkan shader modules"));
            return false;
        }
        if (!createDescriptorSetLayout(device))
        {
            JUTILS_LOG(error, JSTR("Failed to create vulkan descriptor set layout"));
            clearVulkan();
            return false;
        }
        if (!createPipelineLayout(device))
        {
            JUTILS_LOG(error, JSTR("Failed to create vulkan pipeline layout"));
            clearVulkan();
            return false;
        }
        return true;
    }
    bool Shader_Vulkan::createShaderModules(VkDevice device, const jmap<ShaderStageFlags, jstring>& fileNames)
    {
        VkShaderModule modules[2] = { nullptr, nullptr };
        if (!CreateVulkanShaderModule(modules[0], device, fileNames, SHADER_STAGE_VERTEX, ".vert.spv", false))
        {
            JUTILS_LOG(error, JSTR("Failed to create vulkan vertex shader module"));
            return false;
        }
        if (!CreateVulkanShaderModule(modules[1], device, fileNames, SHADER_STAGE_FRAGMENT, ".frag.spv", false))
        {
            JUTILS_LOG(error, JSTR("Failed to create vulkan fragment shader module"));
            vkDestroyShaderModule(device, modules[0], nullptr);
            return false;
        }
        m_ShaderModules = { { SHADER_STAGE_VERTEX, modules[0] }, { SHADER_STAGE_FRAGMENT, modules[1] } };

        m_CachedPipelineStageInfos.reserve(m_ShaderModules.getSize());
        for (const auto& shaderModule : m_ShaderModules)
        {
            VkPipelineShaderStageCreateInfo& stageInfo = m_CachedPipelineStageInfos.addDefault();
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.module = shaderModule.value;
            stageInfo.pName = "main";
            switch (shaderModule.key)
            {
            case SHADER_STAGE_VERTEX: stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
            case SHADER_STAGE_FRAGMENT: stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
            default: ;
            }
            stageInfo.flags = 0;
        }
        return true;
    }
    bool Shader_Vulkan::createDescriptorSetLayout(VkDevice device)
    {
        const jmap<jstringID, ShaderUniform>& uniforms = getUniforms();
        if (uniforms.isEmpty())
        {
            return true;
        }

        jarray<VkDescriptorSetLayoutBinding> layoutBindings;
        jmap<uint32, int32> layoutBindingsMap;
        for (const auto& uniform : uniforms)
        {
            int32 index;
            const int32* indexPtr = layoutBindingsMap.find(uniform.value.shaderLocation);
            if (indexPtr == nullptr)
            {
                index = layoutBindings.getSize();
                layoutBindings.addDefault().stageFlags = 0;
                layoutBindingsMap.add(uniform.value.shaderLocation, index);
            }
            else
            {
                index = *indexPtr;
            }

            VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings[index];
            layoutBinding.binding = uniform.value.shaderLocation;
            layoutBinding.pImmutableSamplers = nullptr;
            if (uniform.value.shaderStages & SHADER_STAGE_VERTEX)
            {
                layoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
            }
            if (uniform.value.shaderStages & SHADER_STAGE_FRAGMENT)
            {
                layoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            layoutBinding.descriptorType = IsShaderUniformScalar(uniform.value.type) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBinding.descriptorCount = 1;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32>(layoutBindings.getSize());
        layoutInfo.pBindings = layoutBindings.getData();
        const VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_DescriptorSetLayout);
        if (result != VK_SUCCESS)
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create vulkan descriptor set layout"));
            return false;
        }
        return true;
    }
    bool Shader_Vulkan::createPipelineLayout(VkDevice device)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        if (m_DescriptorSetLayout != nullptr)
        {
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
        }
        else
        {
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pSetLayouts = nullptr;
        }
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        const VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        if (result != VK_SUCCESS)
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create vulkan pipeline layout"));
            return false;
        }
        return true;
    }

    void Shader_Vulkan::clearAsset()
    {
        clearVulkan();
        Super::clearAsset();
    }
    void Shader_Vulkan::clearVulkan()
    {
        VkDevice device = getRenderEngine<RenderEngine_Vulkan>()->getDevice();

        m_CachedPipelineStageInfos.clear();

        for (const auto& pipeline : m_RenderPipelines)
        {
            vkDestroyPipeline(device, pipeline.value, nullptr);
        }
        m_RenderPipelines.clear();
        if (m_PipelineLayout != nullptr)
        {
            vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
            m_PipelineLayout = nullptr;
        }
        if (m_DescriptorSetLayout != nullptr)
        {
            vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
            m_DescriptorSetLayout = nullptr;
        }
        for (const auto& shaderModule : m_ShaderModules)
        {
            if (shaderModule.value != nullptr)
            {
                vkDestroyShaderModule(device, shaderModule.value, nullptr);
            }
        }
        m_ShaderModules.clear();
    }

    bool Shader_Vulkan::bindRenderPipeline(VkCommandBuffer commandBuffer, const jstringID& vertexName, const VulkanRenderPass* renderPass, 
        const MaterialProperties& pipelineProperties)
    {
        VkPipeline renderPipeline = getRenderPipeline(vertexName, renderPass, pipelineProperties);
        if (renderPipeline == nullptr)
        {
            return false;
        }
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPipeline);
        return true;
    }
    VkPipeline Shader_Vulkan::getRenderPipeline(const jstringID& vertexName, const VulkanRenderPass* renderPass, 
        const MaterialProperties& pipelineProperties)
    {
        if ((vertexName == jstringID_NONE) || (renderPass == nullptr))
        {
            JUTILS_LOG(warning, JSTR("Invalid input params"));
            return nullptr;
        }

        const render_pass_type_id renderPassID = renderPass->getTypeID();
        const RenderPipelineID pipelineID = { vertexName, renderPassID, pipelineProperties };
        VkPipeline* existingPipeline = m_RenderPipelines.find(pipelineID);
        if (existingPipeline != nullptr)
        {
            return *existingPipeline;
        }

        const RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const VulkanRenderPassDescription* renderPassDescription = renderEngine->findRenderPassDescription(renderPassID);
        if (renderPassDescription == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Invalid render pass type"));
            return nullptr;
        }
        const VertexDescription_Vulkan* vertexDescription = renderEngine->findVertexType_Vulkan(vertexName);
        if (vertexDescription == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Invalid vertex type"));
            return nullptr;
        }

        // Vertex input data
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexDescription->binding;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32>(vertexDescription->attributes.getSize());
        vertexInputInfo.pVertexAttributeDescriptions = vertexDescription->attributes.getData();

        // Geometry type
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Depth
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = pipelineProperties.depthEnabled ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable = pipelineProperties.depthEnabled ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = pipelineProperties.stencilEnabled ? VK_TRUE : VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = pipelineProperties.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = pipelineProperties.cullBackFaces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_TRUE;
        multisampling.rasterizationSamples = renderPassDescription->sampleCount;
        multisampling.minSampleShading = 0.2f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        // Blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = pipelineProperties.blendEnabled ? VK_TRUE : VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        // Dynamic states
        constexpr VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32>(m_CachedPipelineStageInfos.getSize());
        pipelineInfo.pStages = m_CachedPipelineStageInfos.getData();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = renderPass->get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = nullptr;
        pipelineInfo.basePipelineIndex = -1;
        VkPipeline renderPipeline;
        const VkResult result = vkCreateGraphicsPipelines(renderEngine->getDevice(), nullptr, 1, &pipelineInfo, nullptr, &renderPipeline);
        if (result != VK_SUCCESS)
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create vulkan render pipeline"));
            return nullptr;
        }

        return m_RenderPipelines[pipelineID] = renderPipeline;
    }
}

#endif

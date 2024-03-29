﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "JumaRE/material/Shader.h"

#include "vulkanObjects/VulkanRenderPassDescription.h"
#include "JumaRE/material/MaterialProperties.h"
#include "JumaRE/vertex/VertexDescription.h"

namespace JumaRenderEngine
{
    class VulkanRenderPass;

    class Shader_Vulkan final : public Shader
    {
        using Super = Shader;

    public:
        Shader_Vulkan() = default;
        virtual ~Shader_Vulkan() override;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return m_DescriptorSetLayout; }
        VkPipelineLayout getPipelineLayout() const { return m_PipelineLayout; }

        bool bindRenderPipeline(VkCommandBuffer commandBuffer, vertex_id vertexID, const VulkanRenderPass* renderPass, 
            const MaterialProperties& pipelineProperties);

    protected:

        virtual bool initInternal(const jmap<ShaderStageFlags, jstring>& fileNames) override;
        virtual void onClearAsset() override;

    private:

        struct RenderPipelineID
        {
            vertex_id vertexID = vertex_id_NONE;
            render_pass_type_id renderPassID = render_pass_type_id_INVALID;
            MaterialProperties properties;

            inline bool operator<(const RenderPipelineID& otherID) const;
        };

        jmap<ShaderStageFlags, VkShaderModule> m_ShaderModules;
        VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
        VkPipelineLayout m_PipelineLayout = nullptr;

        jarray<VkPipelineShaderStageCreateInfo> m_CachedPipelineStageInfos;
        jmap<RenderPipelineID, VkPipeline> m_RenderPipelines;


        bool createShaderModules(VkDevice device, const jmap<ShaderStageFlags, jstring>& fileNames);
        bool createDescriptorSetLayout(VkDevice device);
        bool createPipelineLayout(VkDevice device);

        void clearVulkan();
        
        VkPipeline getRenderPipeline(vertex_id vertexID, const VulkanRenderPass* renderPass, const MaterialProperties& pipelineProperties);
    };

    inline bool Shader_Vulkan::RenderPipelineID::operator<(const RenderPipelineID& otherID) const
    {
        if (vertexID != otherID.vertexID)
        {
            return vertexID < otherID.vertexID;
        }
        if (renderPassID != otherID.renderPassID)
        {
            return renderPassID < otherID.renderPassID;
        }
        return properties < otherID.properties;
    }
}

#endif

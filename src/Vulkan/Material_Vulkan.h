// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../include/JumaRE/material/Material.h"

#include <vulkan/vulkan_core.h>

#include "vulkanObjects/VulkanRenderPassDescription.h"

namespace JumaRenderEngine
{
    class VulkanRenderPass;
    class VulkanBuffer;
    class VertexBuffer_Vulkan;
    struct RenderOptions;

    class Material_Vulkan final : public Material
    {
        using Super = Material;

    public:
        Material_Vulkan() = default;
        virtual ~Material_Vulkan() override;

        bool bindMaterial(const RenderOptions* renderOptions, VertexBuffer_Vulkan* vertexBuffer);
        void unbindMaterial(const RenderOptions* renderOptions, VertexBuffer_Vulkan* vertexBuffer) {}

    protected:

        virtual bool initInternal() override;

    private:

        struct VulkanRenderPipelineID
        {
            jstringID vertexName = jstringID_NONE;
            render_pass_type_id renderPassID = render_pass_type_id_INVALID;

            bool operator<(const VulkanRenderPipelineID& ID) const
            {
                return (vertexName < ID.vertexName) || ((vertexName == ID.vertexName) && (renderPassID < ID.renderPassID));
            }
        };
        
        VkDescriptorPool m_DescriptorPool = nullptr;
        VkDescriptorSet m_DescriptorSet = nullptr;
        jmap<VulkanRenderPipelineID, VkPipeline> m_RenderPipelines;

        jmap<uint32, VulkanBuffer*> m_UniformBuffers;

        
        bool createDescriptorSet();
        bool initDescriptorSetData();
        bool updateDescriptorSetData();

        void clearVulkan();

        bool bindRenderPipeline(VkCommandBuffer commandBuffer, const jstringID& vertexName, const VulkanRenderPass* renderPass);
        bool getRenderPipeline(const jstringID& vertexName, const VulkanRenderPass* renderPass, VkPipeline& outPipeline);

        bool bindDescriptorSet(VkCommandBuffer commandBuffer);
    };
}

#endif

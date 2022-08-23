// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../include/JumaRE/material/Material.h"

#include <vulkan/vulkan_core.h>

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

        VkDescriptorPool m_DescriptorPool = nullptr;
        VkDescriptorSet m_DescriptorSet = nullptr;

        jmap<uint32, VulkanBuffer*> m_UniformBuffers;

        
        bool createDescriptorSet();
        bool initDescriptorSetData();
        bool updateDescriptorSetData();

        void clearVulkan();

        bool bindDescriptorSet(VkCommandBuffer commandBuffer);
    };
}

#endif

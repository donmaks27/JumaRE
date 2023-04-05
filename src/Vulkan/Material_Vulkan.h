// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "JumaRE/material/Material.h"

#include <jutils/jasync_task_queue.h>
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

        virtual bool isReadyForDestroy() const { return !m_CreateTaskActive; }
        virtual void onClearAsset() override;

    private:

        class MaterialCreateTask : public jasync_task
        {
        public:
            MaterialCreateTask() = delete;
            MaterialCreateTask(Material_Vulkan* material) : m_Material(material) {}
            virtual ~MaterialCreateTask() override { m_Material->m_CreateTaskActive = false; }

            virtual void run() override { m_Material->createDescriptorSet(); }

        private:

            Material_Vulkan* m_Material = nullptr;
        };

        jmap<uint32, VulkanBuffer*> m_UniformBuffers;

        VkDescriptorPool m_DescriptorPool = nullptr;
        VkDescriptorSet m_DescriptorSet = nullptr;

        std::atomic_bool m_CreateTaskActive = false;
        bool m_MaterialValid = true;
        bool m_MaterialCreated = false;

        
        bool createDescriptorSet();
        bool initDescriptorSetData();
        bool updateDescriptorSetData();

        void clearVulkan();

        bool bindDescriptorSet(VkCommandBuffer commandBuffer);
    };
}

#endif

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../../include/JumaRE/RenderEngineContextObject.h"

#include <jutils/jlist.h>

#include "VulkanCommandBuffer.h"
#include "VulkanQueueType.h"

namespace JumaRenderEngine
{
    class RenderEngine_Vulkan;

    class VulkanCommandPool final : public RenderEngineContextObjectBase
    {
        friend RenderEngine_Vulkan;

    public:
        VulkanCommandPool() = default;
        virtual ~VulkanCommandPool() override;

        VkCommandPool get() const { return m_CommandPool; }
        VulkanQueueType getQueueType() const { return m_QueueType; }

        VulkanCommandBuffer* getCommandBuffer();
        void returnCommandBuffer(VulkanCommandBuffer* commandBuffer);

    private:

        VkCommandPool m_CommandPool = nullptr;
        VulkanQueueType m_QueueType = VulkanQueueType::Graphics;

        jlist<VulkanCommandBuffer> m_CommandBuffers;
        jlist<VulkanCommandBuffer*> m_UnusedCommandBuffers;


        bool init(VulkanQueueType queueType, VkCommandPoolCreateFlags flags = 0);

        void clearVulkan();

        bool createCommandBuffer(bool primaryLevel, VulkanCommandBuffer& outCommandBuffers);
    };
}

#endif

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../../include/JumaRE/core.h"

#include <vulkan/vulkan_core.h>

namespace JumaRenderEngine
{
    class VulkanCommandPool;

    class VulkanCommandBuffer
    {
        friend VulkanCommandPool;

    public:
        VulkanCommandBuffer() = default;
        ~VulkanCommandBuffer() = default;

        VkCommandBuffer get() const { return m_CommandBuffer; }

        bool submit(bool waitForFinish);
        bool submit(VkSubmitInfo submitInfo, VkFence fenceOnFinish, bool waitForFinish);

        void returnToCommandPool();

    private:

        VulkanCommandPool* m_CommandPool = nullptr;
        VkCommandBuffer m_CommandBuffer = nullptr;
    };
}

#endif

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../../include/JumaRE/core.h"

#include <vulkan/vulkan_core.h>
#include <jutils/jarray.h>
#include <jutils/jmap.h>

namespace JumaRenderEngine
{
    class VulkanImage;
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
        
        void changeImageLayout(VulkanImage* image, VkImageLayout layout);
        void applyBarriers();

        void copyImage(VulkanImage* srcImage, VulkanImage* dstImage);
        void generateMipmaps(VulkanImage* image, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    private:

        VulkanCommandPool* m_CommandPool = nullptr;
        VkCommandBuffer m_CommandBuffer = nullptr;

        jarray<VkImageMemoryBarrier2> m_ImageBarriers;
        jmap<VulkanImage*, VkImageLayout> m_LastImageLayouts;


        VkImageLayout getLastImageLayout(VulkanImage* image);
    };
}

#endif

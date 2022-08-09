// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN)

#include "VulkanCommandBuffer.h"

#include "VulkanCommandPool.h"
#include "../RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    inline bool GetImageLayoutTransitionParams(const VkImageLayout layout, VkAccessFlags2& outAccess, VkPipelineStageFlags2& outStage)
    {
        switch (layout)
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            outAccess = VK_ACCESS_2_NONE;
            outStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            outAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
            outStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            outAccess = VK_ACCESS_2_SHADER_READ_BIT;
            outStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            outAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
            outStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            outAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            outStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            break;
        default: return false;
        }
        return true;
    }
    inline bool FillImageLayoutTransitionParams(VkImageMemoryBarrier2& barrier, const VkImageLayout oldLayout, const VkImageLayout newLayout)
    {
        VkAccessFlags2 srcAccess, dstAccess;
        VkPipelineStageFlags2 srcStage, dstStage;
        if (!GetImageLayoutTransitionParams(oldLayout, srcAccess, srcStage))
        {
            JUTILS_LOG(warning, JSTR("Unsupported src image layout"));
            return false;
        }
        if ((newLayout == VK_IMAGE_LAYOUT_UNDEFINED) || !GetImageLayoutTransitionParams(newLayout, dstAccess, dstStage))
        {
            JUTILS_LOG(warning, JSTR("Unsupported dst image layout"));
            return false;
        }

        barrier.srcStageMask = srcStage;
        barrier.srcAccessMask = srcAccess;
        barrier.dstStageMask = dstStage;
        barrier.dstAccessMask = dstAccess;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        return true;
    }

    bool VulkanCommandBuffer::submit(const bool waitForFinish)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.signalSemaphoreCount = 0;
        return submit(submitInfo, nullptr, waitForFinish);
    }
    bool VulkanCommandBuffer::submit(VkSubmitInfo submitInfo, VkFence fenceOnFinish, const bool waitForFinish)
    {
        const VulkanQueueDescription* queueDescription = m_CommandPool->getRenderEngine<RenderEngine_Vulkan>()->getQueue(m_CommandPool->getQueueType());
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffer;
        const VkResult result = vkQueueSubmit(queueDescription->queue, 1, &submitInfo, fenceOnFinish);
        if (result != VK_SUCCESS)
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to submit command buffer"));
            return false;
        }

        for (const auto& imageLayout : m_LastImageLayouts)
        {
            imageLayout.key->setLayout(imageLayout.value);
        }
        m_LastImageLayouts.clear();

        if (waitForFinish)
        {
            vkQueueWaitIdle(queueDescription->queue);
        }
        return true;
    }

    void VulkanCommandBuffer::returnToCommandPool()
    {
        m_CommandPool->returnCommandBuffer(this);
    }

    VkImageLayout VulkanCommandBuffer::getLastImageLayout(VulkanImage* image)
    {
        const VkImageLayout* lastLayoutPtr = m_LastImageLayouts.find(image);
        if (lastLayoutPtr != nullptr)
        {
            return *lastLayoutPtr;
        }
        return (image != nullptr) ? image->getLayout() : VK_IMAGE_LAYOUT_UNDEFINED;
    }

    void VulkanCommandBuffer::changeImageLayout(VulkanImage* image, const VkImageLayout layout)
    {
        if ((image == nullptr) || !image->isValid() || (layout == VK_IMAGE_LAYOUT_UNDEFINED))
        {
            return;
        }
        const VkImageLayout lastImageLayout = getLastImageLayout(image);
        if (lastImageLayout == layout)
        {
            return;
        }

        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.pNext = nullptr;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image->get();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = image->getMipLevelsCount();
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        if (!FillImageLayoutTransitionParams(barrier, lastImageLayout, layout))
        {
            JUTILS_LOG(warning, JSTR("Unsupported image layout transition"));
            return;
        }

        m_ImageBarriers.add(barrier);
        m_LastImageLayouts.add(image, layout);
    }

    void VulkanCommandBuffer::applyBarriers()
    {
        if (m_ImageBarriers.isEmpty())
        {
            return;
        }

        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = nullptr;
        dependencyInfo.dependencyFlags = 0;
        dependencyInfo.memoryBarrierCount = 0;
        dependencyInfo.bufferMemoryBarrierCount = 0;
        dependencyInfo.imageMemoryBarrierCount = m_ImageBarriers.getSize();
        dependencyInfo.pImageMemoryBarriers = m_ImageBarriers.getData();
        vkCmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);

        m_ImageBarriers.clear();
    }

    void VulkanCommandBuffer::copyImage(VulkanImage* srcImage, VulkanImage* dstImage)
    {
        const VkImageLayout lastSrcImageLayout = getLastImageLayout(srcImage);
        const VkImageLayout lastDstImageLayout = getLastImageLayout(dstImage);

        const math::uvector2 size = srcImage->getSize();
        VkImageCopy imageCopyInfo{};
        imageCopyInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyInfo.srcSubresource.baseArrayLayer = 0;
        imageCopyInfo.srcSubresource.layerCount = 1;
        imageCopyInfo.srcSubresource.mipLevel = 0;
        imageCopyInfo.srcOffset = { 0, 0, 0 };
        imageCopyInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyInfo.dstSubresource.baseArrayLayer = 0;
        imageCopyInfo.dstSubresource.layerCount = 1;
        imageCopyInfo.dstSubresource.mipLevel = 0;
        imageCopyInfo.dstOffset = { 0, 0, 0 };
        imageCopyInfo.extent = { size.x, size.y, 1 };
        vkCmdCopyImage(m_CommandBuffer, 
            srcImage->get(), lastSrcImageLayout, 
            dstImage->get(), lastDstImageLayout, 
            1, &imageCopyInfo
        );
    }

    void VulkanCommandBuffer::generateMipmaps(VulkanImage* image, const VkImageLayout finalLayout)
    {
        if ((image == nullptr) || !image->isValid())
        {
            return;
        }
        const uint8 mipLevelsCount = image->getMipLevelsCount();
        if (mipLevelsCount <= 1)
        {
            changeImageLayout(image, finalLayout);
            return;
        }

        VkImageMemoryBarrier2 barriers[2];
        VkImageMemoryBarrier2& barrier1 = barriers[0];
        barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier1.pNext = nullptr;
        barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier1.image = image->get();
        barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier1.subresourceRange.baseArrayLayer = 0;
        barrier1.subresourceRange.layerCount = 1;
        /*barrier.srcStageMask = srcStage;
        barrier.srcAccessMask = srcAccess;
        barrier.dstStageMask = dstStage;
        barrier.dstAccessMask = dstAccess;
        barrier.oldLayout = lastImageLayout;
        barrier.newLayout = layout;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = image->getMipLevelsCount();*/
        VkImageMemoryBarrier2& barrier2 = barriers[1];
        barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier2.pNext = nullptr;
        barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier2.image = image->get();
        barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier2.subresourceRange.baseArrayLayer = 0;
        barrier2.subresourceRange.layerCount = 1;
        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = nullptr;
        dependencyInfo.dependencyFlags = 0;
        dependencyInfo.memoryBarrierCount = 0;
        dependencyInfo.bufferMemoryBarrierCount = 0;
        dependencyInfo.pImageMemoryBarriers = barriers;
        //dependencyInfo.imageMemoryBarrierCount = 1;

        const VkImageLayout prevImageLayout = getLastImageLayout(image);
        if (prevImageLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            FillImageLayoutTransitionParams(barrier1, prevImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            barrier1.subresourceRange.baseMipLevel = 0;
            barrier1.subresourceRange.levelCount = 1;
            if (prevImageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                FillImageLayoutTransitionParams(barrier2, prevImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                barrier2.subresourceRange.baseMipLevel = 1;
                barrier2.subresourceRange.levelCount = mipLevelsCount - 1;

                dependencyInfo.imageMemoryBarrierCount = 2;
                vkCmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);
            }
            else
            {
                dependencyInfo.imageMemoryBarrierCount = 1;
                vkCmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);
            }
        }
        else
        {
            FillImageLayoutTransitionParams(barrier1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            barrier1.subresourceRange.baseMipLevel = 1;
            barrier1.subresourceRange.levelCount = mipLevelsCount - 1;

            dependencyInfo.imageMemoryBarrierCount = 1;
            vkCmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);
        }

        math::ivector2 mipSize = image->getSize();
        VkImage imageObject = image->get();
        for (uint8 mipLevelIndex = 1; mipLevelIndex < mipLevelsCount; mipLevelIndex++)
        {
            const math::ivector2 nextMipSize = { mipSize.x > 1 ? mipSize.x / 2 : 1, mipSize.y > 1 ? mipSize.y / 2 : 1 };

            VkImageBlit imageBlit{};
            imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.srcSubresource.mipLevel = mipLevelIndex - 1;
            imageBlit.srcSubresource.baseArrayLayer = 0;
            imageBlit.srcSubresource.layerCount = 1;
            imageBlit.srcOffsets[0] = { 0, 0, 0 };
            imageBlit.srcOffsets[1] = { mipSize.x, mipSize.y, 1 };
            imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.dstSubresource.mipLevel = mipLevelIndex;
            imageBlit.dstSubresource.baseArrayLayer = 0;
            imageBlit.dstSubresource.layerCount = 1;
            imageBlit.dstOffsets[0] = { 0, 0, 0 };
            imageBlit.dstOffsets[1] = { nextMipSize.x, nextMipSize.y, 1 };
            vkCmdBlitImage(m_CommandBuffer,
                imageObject, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                imageObject, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &imageBlit, VK_FILTER_LINEAR
            );
            if (mipLevelIndex != (mipLevelsCount - 1))
            {
                FillImageLayoutTransitionParams(barrier1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
                barrier1.subresourceRange.baseMipLevel = mipLevelIndex;
                barrier1.subresourceRange.levelCount = 1;

                dependencyInfo.imageMemoryBarrierCount = 1;
                vkCmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);

                mipSize = nextMipSize;
            }
        }

        if (finalLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            FillImageLayoutTransitionParams(barrier1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, finalLayout);
            barrier1.subresourceRange.baseMipLevel = 0;
            barrier1.subresourceRange.levelCount = mipLevelsCount - 1;
            m_ImageBarriers.add(barrier1);
            if (finalLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                FillImageLayoutTransitionParams(barrier2, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, finalLayout);
                barrier2.subresourceRange.baseMipLevel = mipLevelsCount - 1;
                barrier2.subresourceRange.levelCount = 1;
                m_ImageBarriers.add(barrier2);
            }
        }
        else
        {
            FillImageLayoutTransitionParams(barrier1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            barrier1.subresourceRange.baseMipLevel = mipLevelsCount - 1;
            barrier1.subresourceRange.levelCount = 1;
            m_ImageBarriers.add(barrier1);
        }

        m_LastImageLayouts.add(image, finalLayout);
    }
}

#endif

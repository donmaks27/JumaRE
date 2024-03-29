﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN)

#include "VulkanImage.h"

#include "VulkanBuffer.h"
#include "VulkanCommandPool.h"
#include "../RenderEngine_Vulkan.h"
#include "../TextureFormat_Vulkan.h"
#include "../../../include/JumaRE/texture/TextureBase.h"

namespace JumaRenderEngine
{
    VulkanImage::~VulkanImage()
    {
        clearVulkan();
    }

    void VulkanImage::clearVulkan()
    {
        const RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();

        if (m_ImageView != nullptr)
        {
            vkDestroyImageView(renderEngine->getDevice(), m_ImageView, nullptr);
            m_ImageView = nullptr;
        }
        if (m_Allocation != nullptr)
        {
            vmaDestroyImage(renderEngine->getAllocator(), m_Image, m_Allocation);
            m_Allocation = nullptr;
        }
        m_Image = nullptr;

        m_Size = { 0, 0 };
        m_Format = VK_FORMAT_UNDEFINED;
    }

    bool VulkanImage::init(const VkImageUsageFlags usage, const std::initializer_list<VulkanQueueType> accessedQueues,
        const math::uvector2& size, const VkSampleCountFlagBits sampleCount, const VkFormat format, const uint8 mipLevels)
    {
        if (isValid())
        {
            JUTILS_LOG(warning, JSTR("Vulkan image already initialized"));
            return false;
        }

        const RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        jarray<uint32> accessedQueueFamilies;
        accessedQueueFamilies.reserve(static_cast<int32>(accessedQueues.size()));
        for (const auto& queue : accessedQueues)
        {
            accessedQueueFamilies.addUnique(renderEngine->getQueue(queue)->familyIndex);
        }

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = size.x;
        imageInfo.extent.height = size.y;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        if (accessedQueueFamilies.getSize() > 1)
        {
            imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
            imageInfo.queueFamilyIndexCount = static_cast<uint32>(accessedQueueFamilies.getSize());
            imageInfo.pQueueFamilyIndices = accessedQueueFamilies.getData();
        }
        else
        {
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.queueFamilyIndexCount = 0;
            imageInfo.pQueueFamilyIndices = nullptr;
        }
        imageInfo.samples = sampleCount;
        imageInfo.flags = 0;
        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.flags = 0;
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.preferredFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        const VkResult result = vmaCreateImage(renderEngine->getAllocator(), &imageInfo, &allocationInfo, &m_Image, &m_Allocation, nullptr);
        if (result != VK_SUCCESS)
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create vulkan image"));
            return false;
        }

        m_Size = size;
        m_Format = format;
        m_MipLevels = mipLevels;
        m_Layout = imageInfo.initialLayout;
        markAsInitialized();
        return true;
    }
    bool VulkanImage::init(const VkImageUsageFlags usage, const std::initializer_list<VulkanQueueType> accessedQueues,
        const math::uvector2& size, const VkSampleCountFlagBits sampleCount, const VkFormat format)
    {
        return init(usage, accessedQueues, size, sampleCount, format, GetMipLevelCountByTextureSize(size));
    }
    bool VulkanImage::init(VkImage existingImage, const math::uvector2& size, const VkFormat format, const uint8 mipLevels)
    {
        if (isValid())
        {
            JUTILS_LOG(warning, JSTR("Vulkan image already initialized"));
            return false;
        }
        if ((existingImage == nullptr) || (size.x == 0) || (size.y == 0) || (mipLevels == 0))
        {
            JUTILS_LOG(error, JSTR("Invalid input params"));
            return false;
        }

        m_Image = existingImage;
        m_Allocation = nullptr;
        m_Size = size;
        m_Format = format;
        m_MipLevels = mipLevels;
        markAsInitialized();
        return true;
    }

    bool VulkanImage::createImageView(const VkImageAspectFlags aspectFlags)
    {
        if (!isValid())
        {
            JUTILS_LOG(error, JSTR("Vulkan image not initialized"));
            return false;
        }
        if (m_ImageView != nullptr)
        {
            JUTILS_LOG(warning, JSTR("Vulkan image view already created"));
            return false;
        }

        VkImageViewCreateInfo imageViewInfo{};
	    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	    imageViewInfo.image = m_Image;
	    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	    imageViewInfo.format = m_Format;
	    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imageViewInfo.subresourceRange.aspectMask = aspectFlags;
	    imageViewInfo.subresourceRange.baseMipLevel = 0;
	    imageViewInfo.subresourceRange.levelCount = m_MipLevels;
	    imageViewInfo.subresourceRange.baseArrayLayer = 0;
	    imageViewInfo.subresourceRange.layerCount = 1;
        const VkResult result = vkCreateImageView(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), &imageViewInfo, nullptr, &m_ImageView);
        if (result != VK_SUCCESS)
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create vulkan image view"));
            return false;
        }
        return true;
    }

    void VulkanImage::setLayout(const VkImageLayout layout)
    {
        if (isValid())
        {
            m_Layout = layout;
        }
    }

    bool VulkanImage::setImageData(const uint8* data, const VkImageLayout newLayout)
    {
        if (data == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Invalid input params"));
            return false;
        }
        const uint32 formatSize = GetTextureFormatSize(GetTextureFormatByVulkanFormat(m_Format));
        if (formatSize == 0)
        {
            JUTILS_LOG(warning, JSTR("Unsupported image format"));
            return false;
        }

        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const uint32 imageSize = m_Size.x * m_Size.y * formatSize;
        VulkanBuffer* stagingBuffer = renderEngine->getVulkanBuffer();
        if (!stagingBuffer->initStaging(imageSize) || !stagingBuffer->setData(data, imageSize, 0, true))
        {
            JUTILS_LOG(error, JSTR("Failed to create staging buffer"));
            renderEngine->returnVulkanBuffer(stagingBuffer);
            return false;
        }

        VulkanCommandBuffer* commandBuffer = renderEngine->getCommandPool(VulkanQueueType::Graphics)->getCommandBuffer();
        VkCommandBuffer vulkanCommandBuffer = commandBuffer->get();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(vulkanCommandBuffer, &beginInfo);

        commandBuffer->changeImageLayout(this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        commandBuffer->applyBarriers();

        VkBufferImageCopy imageCopy{};
        imageCopy.bufferOffset = 0;
        imageCopy.bufferRowLength = 0;
        imageCopy.bufferImageHeight = 0;
        imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.imageSubresource.mipLevel = 0;
        imageCopy.imageSubresource.baseArrayLayer = 0;
        imageCopy.imageSubresource.layerCount = 1;
        imageCopy.imageOffset = { 0, 0, 0 };
        imageCopy.imageExtent = { m_Size.x, m_Size.y, 1 };
        vkCmdCopyBufferToImage(vulkanCommandBuffer, stagingBuffer->get(), m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

        commandBuffer->generateMipmaps(this, newLayout);
        commandBuffer->applyBarriers();

        vkEndCommandBuffer(vulkanCommandBuffer);
        if (!commandBuffer->submit(true))
        {
            JUTILS_LOG(error, JSTR("Failed to submit render command buffer"));
            commandBuffer->returnToCommandPool();
            renderEngine->returnVulkanBuffer(stagingBuffer);
            return false;
        }

        commandBuffer->returnToCommandPool();
        renderEngine->returnVulkanBuffer(stagingBuffer);
        return true;
    }
}

#endif

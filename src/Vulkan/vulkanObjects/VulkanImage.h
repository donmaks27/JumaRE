// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../../include/JumaRE/RenderEngineContextObject.h"

#include <vma/vk_mem_alloc.h>
#include <jutils/math/vector2.h>

#include "VulkanQueueType.h"

namespace JumaRenderEngine
{
    class VulkanImage final : public RenderEngineContextObject
    {
    public:
        VulkanImage() = default;
        virtual ~VulkanImage() override;

        bool init(VkImageUsageFlags usage, std::initializer_list<VulkanQueueType> accessedQueues, const math::uvector2& size, 
            VkSampleCountFlagBits sampleCount, VkFormat format, uint8 mipLevels);
        bool init(VkImageUsageFlags usage, std::initializer_list<VulkanQueueType> accessedQueues, const math::uvector2& size, 
            VkSampleCountFlagBits sampleCount, VkFormat format);
        bool init(VkImage existingImage, const math::uvector2& size, VkFormat format, uint8 mipLevels);

        bool createImageView(VkImageAspectFlags aspectFlags);

        VkImage get() const { return m_Image; }
        VkImageView getImageView() const { return m_ImageView; }
        uint8 getMipLevelsCount() const { return m_MipLevels; }

        const math::uvector2& getSize() const { return m_Size; }
        VkImageLayout getLayout() const { return m_Layout; }
        void setLayout(VkImageLayout layout);

        bool setImageData(const uint8* data, VkImageLayout newLayout);

    protected:

        virtual void clearInternal() override { clearVulkan(); }

    private:

        VkImage m_Image = nullptr;
        VmaAllocation m_Allocation = nullptr;
        VkImageView m_ImageView = nullptr;

        math::uvector2 m_Size = { 0, 0 };
        VkFormat m_Format = VK_FORMAT_UNDEFINED;
        uint8 m_MipLevels = 0;

        VkImageLayout m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;


        void clearVulkan();
    };
}

#endif

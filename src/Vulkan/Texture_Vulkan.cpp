// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_VULKAN)

#include "Texture_Vulkan.h"

#include "RenderEngine_Vulkan.h"
#include "TextureFormat_Vulkan.h"
#include "vulkanObjects/VulkanImage.h"

namespace JumaRenderEngine
{
    Texture_Vulkan::~Texture_Vulkan()
    {
        clearVulkan();
    }

    bool Texture_Vulkan::initInternal(const math::uvector2& size, const TextureFormat format, const uint8* data)
    {
        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();

        VulkanImage* image = renderEngine->getVulkanImage();
        const bool imageInitialized = image->init(
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
            { VulkanQueueType::Graphics, VulkanQueueType::Transfer }, size, VK_SAMPLE_COUNT_1_BIT, GetVulkanFormatByTextureFormat(format)
        );
        if (!imageInitialized)
        {
            JUTILS_LOG(error, JSTR("Failed to initialize vulkan image"));
            renderEngine->returnVulkanImage(image);
            return false;
        }

        const bool setImageDataSuccess = image->setImageData(data, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        if (!setImageDataSuccess)
        {
            JUTILS_LOG(error, JSTR("Failed to load data to vulkan image"));
            renderEngine->returnVulkanImage(image);
            return false;
        }
        if (!image->createImageView(VK_IMAGE_ASPECT_COLOR_BIT))
        {
            JUTILS_LOG(error, JSTR("Failed to create vukan image view"));
            renderEngine->returnVulkanImage(image);
            return false;
        }

        m_Image = image;
        return true;
    }

    void Texture_Vulkan::clearVulkan()
    {
        if (m_Image != nullptr)
        {
            getRenderEngine<RenderEngine_Vulkan>()->returnVulkanImage(m_Image);
            m_Image = nullptr;
        }
    }
}

#endif

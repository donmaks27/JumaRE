// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../../include/JumaRE/core.h"

#include <vulkan/vulkan_core.h>

namespace JumaRenderEngine
{
    class VulkanImage;

    struct VulkanFramebufferData
    {
        VkFramebuffer framebuffer = nullptr;

        VulkanImage* colorAttachment = nullptr;
        VulkanImage* depthAttachment = nullptr;
        VulkanImage* resolveAttachment = nullptr;

        VulkanImage* resultImage = nullptr;
    };
}

#endif

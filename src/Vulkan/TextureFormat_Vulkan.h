// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../include/JumaRE/texture/TextureFormat.h"

#include <vulkan/vulkan_core.h>

namespace JumaRenderEngine
{
    constexpr VkFormat GetVulkanFormatByTextureFormat(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA8: return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::BGRA8: return VK_FORMAT_B8G8R8A8_SRGB;
        case TextureFormat::DEPTH32: return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::DEPTH24_STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;
        default: ;
        }
        return VK_FORMAT_UNDEFINED;
    }
}

#endif
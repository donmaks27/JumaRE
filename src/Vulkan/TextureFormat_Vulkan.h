// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "JumaRE/texture/TextureFormat.h"

#include <vulkan/vulkan_core.h>

namespace JumaRenderEngine
{
    constexpr VkFormat GetVulkanFormatByTextureFormat(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::DEPTH32: return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::DEPTH24_STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;
        default: ;
        }
        return VK_FORMAT_UNDEFINED;
    }
    constexpr TextureFormat GetTextureFormatByVulkanFormat(const VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_R8G8B8A8_UNORM: return TextureFormat::RGBA8;
        case VK_FORMAT_R8G8B8A8_SRGB: return TextureFormat::RGBA8_SRGB;
        case VK_FORMAT_D32_SFLOAT: return TextureFormat::DEPTH32;
        case VK_FORMAT_D24_UNORM_S8_UINT: return TextureFormat::DEPTH24_STENCIL8;
        default: ;
        }
        return TextureFormat::NONE;
    }
}

#endif
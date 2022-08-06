// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

namespace JumaRenderEngine
{
    enum class TextureFilterType : uint8
    {
        Point,
        Bilinear,
        Trilinear,
        Anisotropic2,
        Anisotropic4,
        Anisotropic8,
        Anisotropic16
    };
    constexpr uint8 GetTextureFilterTypeCount() { return 7; }

    enum class TextureWrapMode : uint8
    {
        Repeat,
        Mirror,
        Clamp
    };
    constexpr uint8 GetTextureWrapModeCount() { return 3; }

    struct TextureSamplerType
    {
        TextureFilterType filterType = TextureFilterType::Point;
        TextureWrapMode wrapMode = TextureWrapMode::Clamp;

        bool operator<(const TextureSamplerType& type) const { return (filterType < type.filterType) || ((filterType == type.filterType) && (wrapMode < type.wrapMode)); }
    };
    constexpr uint8 GetTextureSamplerTypeCount() { return GetTextureFilterTypeCount() * GetTextureWrapModeCount(); }

    constexpr uint8 GetTextureSamplerTypeID(const TextureSamplerType& samplerType)
    {
        return static_cast<uint8>(static_cast<uint8>(samplerType.filterType) * GetTextureWrapModeCount() + static_cast<uint8>(samplerType.wrapMode));
    }
    constexpr TextureSamplerType GetTextureSamplerType(const uint8 samplerTypeID)
    {
        return {
            static_cast<TextureFilterType>(samplerTypeID / GetTextureWrapModeCount()),
            static_cast<TextureWrapMode>(samplerTypeID % GetTextureWrapModeCount())
        };
    }
}

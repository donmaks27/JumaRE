﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"

namespace JumaRenderEngine
{
    enum class TextureFormat : uint8
    {
        NONE,
        RGBA8,
        RGBA8_SRGB,
        DEPTH32,
        DEPTH24_STENCIL8
    };

    constexpr uint32 GetTextureFormatSize(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA8:
        case TextureFormat::RGBA8_SRGB:
        case TextureFormat::DEPTH32:
        case TextureFormat::DEPTH24_STENCIL8:
            return 4;

        default: ;
        }
        return 0;
    }
}

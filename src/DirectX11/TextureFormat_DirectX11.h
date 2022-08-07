﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "../../include/JumaRE/texture/TextureFormat.h"

#include <dxgiformat.h>

namespace JumaRenderEngine
{
    constexpr DXGI_FORMAT GetDirectX11FormatByTextureFormat(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::BGRA8: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::DEPTH32: return DXGI_FORMAT_D32_FLOAT;
        case TextureFormat::DEPTH24_STENCIL8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
        default: ;
        }
        return DXGI_FORMAT_UNKNOWN;
    }
}

#endif
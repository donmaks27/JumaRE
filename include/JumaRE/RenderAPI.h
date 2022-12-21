// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"

namespace JumaRenderEngine
{
    enum class RenderAPI : uint8
    {
        Vulkan,
        OpenGL,
        DirectX11,
        DirectX12
    };

    constexpr const char* RenderAPIToString(const RenderAPI api)
    {
        switch (api)
        {
        case RenderAPI::Vulkan: return JSTR("Vulkan");
        case RenderAPI::OpenGL: return JSTR("OpenGL");
        case RenderAPI::DirectX11: return JSTR("DirectX11");
        case RenderAPI::DirectX12: return JSTR("DirectX12");
        default: ;
        }
        return JSTR("NONE");
    }
}

JUTILS_STRING_FORMATTER(JumaRenderEngine::RenderAPI, JumaRenderEngine::RenderAPIToString);

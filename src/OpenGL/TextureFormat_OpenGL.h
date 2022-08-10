// Copyright a Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL)

#include "../../include/JumaRE/texture/TextureFormat.h"

#include <GL/glew.h>

namespace JumaRenderEngine
{
    constexpr GLenum GetOpenGLFormatByTextureFormat(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA8: return GL_RGBA;
        case TextureFormat::RGBA8_SRGB: return GL_SRGB_ALPHA;
        case TextureFormat::DEPTH32: return GL_DEPTH_COMPONENT32F;
        case TextureFormat::DEPTH24_STENCIL8: return GL_DEPTH24_STENCIL8;
        default: ;
        }
        return 0;
    }
}

#endif

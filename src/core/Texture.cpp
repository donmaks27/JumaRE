// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "JumaRE/texture/Texture.h"

namespace JumaRenderEngine
{
    bool Texture::init(const math::uvector2& size, const TextureFormat format, const uint8* data)
    {
        if ((size.x == 0) || (size.y == 0) || (data == nullptr))
        {
            JUTILS_LOG(error, JSTR("Invalid input params"));
            return false;
        }

        if (!initInternal(size, format, data))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize texture"));
            return false;
        }
        return true;
    }
}

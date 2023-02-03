// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"
#include "../RenderEngineAsset.h"

#include <jutils/math/vector2.h>

#include "TextureSamplerType.h"

namespace JumaRenderEngine
{
    inline uint8 GetMipLevelCountByTextureSize(const math::uvector2& size)
    {
        return math::roundDown<uint8>(static_cast<float>(std::log2(math::min(size.x, size.y)))) + 1;
    }

    class TextureBase : public RenderEngineAsset
    {
    public:
        TextureBase() = default;
        virtual ~TextureBase() override = default;
        
        math::uvector2 getSize() const { return m_TextureSize; }

        TextureSamplerType getSamplerType() const { return m_Sampler; }
        void setSamplerType(const TextureSamplerType sampler) { m_Sampler = sampler; }

    protected:

        math::uvector2 m_TextureSize = { 0, 0 };

    private:

        TextureSamplerType m_Sampler = { TextureFilterType::Trilinear, TextureWrapMode::Clamp };
    };
}

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_INCLUDE_RENDER_API_OPENGL)

#include <JumaRE/texture/Texture.h>

namespace JumaRenderEngine
{
    class Texture_OpenGL final : public Texture
    {
        using Super = Texture;

    public:
        Texture_OpenGL() = default;
        virtual ~Texture_OpenGL() override;

        bool bindToShader(const uint32 bindIndex) const { return bindToShader(this, m_TextureIndex, bindIndex, getSamplerType()); }
        static bool bindToShader(const RenderEngineContextObjectBase* contextObject, uint32 textureIndex, uint32 bindIndex, TextureSamplerType sampler);
        static void unbindTexture(uint32 bindIndex);

    protected:

        virtual bool initInternal(const math::uvector2& size, TextureFormat format, const uint8* data) override;

    private:

        uint32 m_TextureIndex = 0;


        void clearOpenGL();
    };
}

#endif

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "../../include/JumaRE/texture/Texture.h"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace JumaRenderEngine
{
    class Texture_DirectX11 final : public Texture
    {
        using Super = Texture;

    public:
        Texture_DirectX11() = default;
        virtual ~Texture_DirectX11() override;

        ID3D11ShaderResourceView* getTextureView() const { return m_TextureView; }

    protected:

        virtual bool initInternal(const math::uvector2& size, TextureFormat format, const uint8* data) override;
        virtual void clearAsset() override;

    private:

        ID3D11Texture2D* m_Texture = nullptr;
        ID3D11ShaderResourceView* m_TextureView = nullptr;


        void clearDirectX();
    };
}

#endif

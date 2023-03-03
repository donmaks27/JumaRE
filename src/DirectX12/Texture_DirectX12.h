// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "JumaRE/texture/Texture.h"
#include "DirectX12Objects/DirectX12MipGeneratorTarget.h"

struct ID3D12DescriptorHeap;

namespace JumaRenderEngine
{
    class DirectX12Texture;

    class Texture_DirectX12 final : public Texture, public DirectX12MipGeneratorTarget
    {
        using Super = Texture;

    public:
        Texture_DirectX12() = default;
        virtual ~Texture_DirectX12() override;

        DirectX12Texture* getTextureObject() const { return m_Texture; }
        ID3D12DescriptorHeap* getSRV() const { return m_DescriptorHeapSRV; }

    protected:

        virtual bool initInternal(const math::uvector2& size, TextureFormat format, const uint8* data) override;
        virtual void onClearAsset() override;

        virtual DirectX12Texture* getMipGeneratorTargetTexture() const override { return m_Texture; }

    private:

        DirectX12Texture* m_Texture = nullptr;
        ID3D12DescriptorHeap* m_DescriptorHeapSRV = nullptr;


        void clearDirectX();
    };
}

#endif

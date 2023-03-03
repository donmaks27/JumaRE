// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "JumaRE/RenderTarget.h"

#include <jutils/jarray.h>

#include "DirectX12Objects/DirectX12MipGeneratorTarget.h"

namespace JumaRenderEngine
{
    class DirectX12Swapchain;
    class DirectX12Texture;

    class RenderTarget_DirectX12 final : public RenderTarget, public DirectX12MipGeneratorTarget
    {
        using Super = RenderTarget;

        friend DirectX12Swapchain;

    public:
        RenderTarget_DirectX12() = default;
        virtual ~RenderTarget_DirectX12() override;

        ID3D12DescriptorHeap* getSRV() const { return m_DescriptorHeapSRV; }

        virtual bool onStartRender(RenderOptions* renderOptions) override;
        virtual void onFinishRender(RenderOptions* renderOptions) override;

    protected:

        virtual bool initInternal() override;
        virtual void clearAssetInternal() override;

        virtual DirectX12Texture* getMipGeneratorTargetTexture() const override
        {
            if (!isWindowRenderTarget())
            {
                return !m_ResultTextures.isEmpty() ? m_ResultTextures[0] : m_ColorTexture;
            }
            return nullptr;
        }

        virtual bool recreateRenderTarget() override;

    private:

        DirectX12Texture* m_ColorTexture = nullptr;
        jarray<DirectX12Texture*> m_ResultTextures;
        ID3D12DescriptorHeap* m_DescriptorHeapRTV = nullptr;
        ID3D12DescriptorHeap* m_DescriptorHeapSRV = nullptr;

        DirectX12Texture* m_DepthTexture = nullptr;
        ID3D12DescriptorHeap* m_DescriptorHeapDSV = nullptr;


        bool initWindowRenderTarget();
        bool initRenderTarget();

        void clearDirectX();
        void clearRenderTarget();
    };
}

#endif

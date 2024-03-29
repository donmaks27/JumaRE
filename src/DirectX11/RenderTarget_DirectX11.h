﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "JumaRE/RenderTarget.h"

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11RasterizerState;

namespace JumaRenderEngine
{
    class WindowController_DirectX11;

    class RenderTarget_DirectX11 final : public RenderTarget
    {
        using Super = RenderTarget;

        friend WindowController_DirectX11;

    public:
        RenderTarget_DirectX11() = default;
        virtual ~RenderTarget_DirectX11() override;

        ID3D11ShaderResourceView* getResultImageView() const { return m_ResultImageView; }

        virtual bool onStartRender(RenderOptions* renderOptions) override;
        virtual void onFinishRender(RenderOptions* renderOptions) override;

    protected:

        virtual bool initInternal() override;
        virtual void clearAssetInternal() override;

        virtual bool recreateRenderTarget() override;

    private:

        ID3D11Texture2D* m_ColorAttachmentImage = nullptr;
        ID3D11Texture2D* m_DepthAttachmentImage = nullptr;
        ID3D11Texture2D* m_ResolveAttachmentImage = nullptr;
        ID3D11RenderTargetView* m_ColorAttachmentView = nullptr;
        ID3D11DepthStencilView* m_DepthAttachmentView = nullptr;
        ID3D11ShaderResourceView* m_ResultImageView = nullptr;


        bool initWindowRenderTarget();
        bool initRenderTarget(ID3D11Texture2D* resultImage);

        void clearDirectX11();
        void clearRenderTarget();
    };
}

#endif

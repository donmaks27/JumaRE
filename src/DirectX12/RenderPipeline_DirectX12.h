// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "../../include/JumaRE/RenderPipeline.h"

namespace JumaRenderEngine
{
    class DirectX12CommandList;

    class RenderPipeline_DirectX12 final : public RenderPipeline
    {
        using Super = RenderPipeline;

    public:
        RenderPipeline_DirectX12() = default;
        virtual ~RenderPipeline_DirectX12() override;

        virtual void waitForRenderFinished() override;

    protected:

        virtual void renderInternal() override;

        virtual bool onStartRender(RenderOptions* renderOptions) override;
        virtual bool onStartRenderToRenderTarget(RenderOptions* renderOptions, RenderTarget* renderTarget) override;
        virtual void onFinishRenderToRenderTarget(RenderOptions* renderOptions, RenderTarget* renderTarget) override;
        virtual void onFinishRender(RenderOptions* renderOptions) override;

    private:

        DirectX12CommandList* m_RenderCommandList = nullptr;


        void clearDirectX();

        void waitForPreviousRenderFinish();
        bool startCommandListRecord(RenderOptions* renderOptions);
        void finishCommandListRecord(RenderOptions* renderOptions);
    };
}

#endif

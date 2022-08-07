﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "../../../include/JumaRE/RenderEngineContextObject.h"

#include <d3d12.h>

namespace JumaRenderEngine
{
    class DirectX12CommandList;
    class DirectX12MipGeneratorTarget;
    class RenderEngine_DirectX12;

    class DirectX12MipGenerator : public RenderEngineContextObjectBase
    {
        friend RenderEngine_DirectX12;

    public:
        DirectX12MipGenerator() = default;
        virtual ~DirectX12MipGenerator() override;

        void generateMips(DirectX12CommandList* commandListObject, DirectX12MipGeneratorTarget* target, D3D12_RESOURCE_STATES finalState) const;

    private:

        ID3D12RootSignature* m_ShaderRootSignature = nullptr;
        ID3D12PipelineState* m_ShaderPipelineState = nullptr;


        bool init();

        void clearDirectX();
    };
}

#endif

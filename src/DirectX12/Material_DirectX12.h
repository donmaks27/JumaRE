// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "../../include/JumaRE/material/Material.h"

#include "../../include/JumaRE/texture/TextureFormat.h"
#include "../../include/JumaRE/texture/TextureSamples.h"

struct ID3D12PipelineState;
struct ID3D12DescriptorHeap;

namespace JumaRenderEngine
{
    struct RenderOptions;
    class VertexBuffer_DirectX12;
    class DirectX12Buffer;

    class Material_DirectX12 final : public Material
    {
    public:
        Material_DirectX12() = default;
        virtual ~Material_DirectX12() override;

        bool bindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX12* vertexBuffer);
        void unbindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX12* vertexBuffer) {}

    protected:

        virtual bool initInternal() override;

    private:

        struct PipelineStateID
        {
            jstringID vertexName = jstringID_NONE;
            TextureFormat colorFormat = TextureFormat::RGBA8;
            TextureFormat depthFormat = TextureFormat::DEPTH24_STENCIL8;
            TextureSamples samplesCount = TextureSamples::X1;

            bool operator<(const PipelineStateID& ID) const
            {
                return (vertexName < ID.vertexName) || (colorFormat < ID.colorFormat) || (depthFormat < ID.depthFormat) || (samplesCount < ID.samplesCount);
            }
        };

        jmap<PipelineStateID, ID3D12PipelineState*> m_PipelineStates;

        jmap<uint32, DirectX12Buffer*> m_UniformBuffers;
        ID3D12DescriptorHeap* m_TextureDescriptorHeap = nullptr;
        ID3D12DescriptorHeap* m_SamplerDescriptorHeap = nullptr;


        void clearDirectX();

        bool getPipelineState(const PipelineStateID& pipelineStateID, ID3D12PipelineState*& outPipelineState);

        bool updateUniformData();
    };
}

#endif

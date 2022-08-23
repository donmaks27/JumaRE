// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "../../include/JumaRE/material/Material.h"

struct ID3D12DescriptorHeap;

namespace JumaRenderEngine
{
    struct RenderOptions_DirectX12;
    class VertexBuffer_DirectX12;
    class DirectX12Buffer;

    class Material_DirectX12 final : public Material
    {
    public:
        Material_DirectX12() = default;
        virtual ~Material_DirectX12() override;

        bool bindMaterial(const RenderOptions_DirectX12* renderOptions, VertexBuffer_DirectX12* vertexBuffer);
        void unbindMaterial(const RenderOptions_DirectX12* renderOptions, VertexBuffer_DirectX12* vertexBuffer) {}

    protected:

        virtual bool initInternal() override;

    private:

        jmap<uint32, DirectX12Buffer*> m_UniformBuffers;
        ID3D12DescriptorHeap* m_TextureDescriptorHeap = nullptr;
        ID3D12DescriptorHeap* m_SamplerDescriptorHeap = nullptr;


        void clearDirectX();

        bool updateUniformData();
    };
}

#endif

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "../../include/JumaRE/material/Material.h"

struct ID3D11Buffer;
struct ID3D11DeviceContext;

namespace JumaRenderEngine
{
    class VertexBuffer_DirectX11;
    struct RenderOptions;

    class Material_DirectX11 final : public Material
    {
        using Super = Material;

    public:
        Material_DirectX11() = default;
        virtual ~Material_DirectX11() override;

        bool bindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer);
        void unbindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer);

    protected:

        virtual bool initInternal() override;

    private:

        struct UniformBufferDescription
        {
            ID3D11Buffer* buffer = nullptr;
            uint8 shaderStages = 0;
        };

        jmap<uint32, UniformBufferDescription> m_UniformBuffers;


        void clearDirectX();

        void bindUniforms(ID3D11DeviceContext* deviceContext);
        void updateUniformBuffersData(ID3D11DeviceContext* deviceContext);
    };
}

#endif

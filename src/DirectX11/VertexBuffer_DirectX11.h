// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "../../include/JumaRE/vertex/VertexBuffer.h"

struct ID3D11Buffer;

namespace JumaRenderEngine
{
    class VertexBuffer_DirectX11 final : public VertexBuffer
    {
        using Super = VertexBuffer;

    public:
        VertexBuffer_DirectX11() = default;
        virtual ~VertexBuffer_DirectX11() override;

        virtual void render(const RenderOptions* renderOptions, Material* material) override;

    protected:

        virtual bool initInternal(const VertexBufferData& data) override;
        virtual void clearAsset() override;

    private:

        ID3D11Buffer* m_VertexBuffer = nullptr;
        ID3D11Buffer* m_IndexBuffer = nullptr;

        uint32 m_RenderElementsCount = 0;
        uint32 m_VertexSize = 0;


        void clearDirectX();
    };
}

#endif

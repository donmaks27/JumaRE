// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "JumaRE/vertex/VertexBuffer.h"

namespace JumaRenderEngine
{
    class DirectX12Buffer;

    class VertexBuffer_DirectX12 final : public VertexBuffer
    {
        using Super = VertexBuffer;

    public:
        VertexBuffer_DirectX12() = default;
        virtual ~VertexBuffer_DirectX12() override;

        virtual void render(const RenderOptions* renderOptions, Material* material) override;

    protected:

        virtual bool initInternal(const VertexBufferData& data) override;
        virtual void onClearAsset() override;

    private:

        DirectX12Buffer* m_VertexBuffer = nullptr;
        DirectX12Buffer* m_IndexBuffer = nullptr;

        uint32 m_CachedVertexSize = 0;
        uint32 m_RenderElementsCount = 0;


        void clearDirectX();
    };
}

#endif

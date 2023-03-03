// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX12)

#include "VertexBuffer_DirectX12.h"

#include "Material_DirectX12.h"
#include "RenderEngine_DirectX12.h"
#include "RenderOptions_DirectX12.h"
#include "JumaRE/vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    VertexBuffer_DirectX12::~VertexBuffer_DirectX12()
    {
        clearDirectX();
    }

    bool VertexBuffer_DirectX12::initInternal(const VertexBufferData& data)
    {
        if (data.vertexCount == 0)
        {
            JUTILS_LOG(error, JSTR("Empty vertex buffer data"));
            return false;
        }
        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        const uint32 vertexSize = renderEngine->findVertex(getVertexID())->vertexSize;

        DirectX12Buffer* vertexBuffer = renderEngine->getBuffer();
        if ((vertexBuffer == nullptr) || !vertexBuffer->initGPU(vertexSize * data.vertexCount, data.verticesData, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER))
        {
            JUTILS_LOG(error, JSTR("Failed to create vertex buffer"));
            renderEngine->returnBuffer(vertexBuffer);
            return false;
        }

        DirectX12Buffer* indexBuffer = nullptr;
        if (data.indexCount > 0)
        {
            indexBuffer = renderEngine->getBuffer();
            if ((indexBuffer == nullptr) || !indexBuffer->initGPU(sizeof(uint32) * data.indexCount, data.indicesData, D3D12_RESOURCE_STATE_INDEX_BUFFER))
            {
                JUTILS_LOG(error, JSTR("Failed to create index buffer"));
                renderEngine->returnBuffer(indexBuffer);
                renderEngine->returnBuffer(vertexBuffer);
                return false;
            }
        }

        m_VertexBuffer = vertexBuffer;
        m_IndexBuffer = indexBuffer;
        m_CachedVertexSize = vertexSize;
        m_RenderElementsCount = m_IndexBuffer != nullptr ? data.indexCount : data.vertexCount;
        return true;
    }

    void VertexBuffer_DirectX12::onClearAsset()
    {
        clearDirectX();
        Super::onClearAsset();
    }
    void VertexBuffer_DirectX12::clearDirectX()
    {
        if (m_VertexBuffer != nullptr)
        {
            RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();

            renderEngine->returnBuffer(m_VertexBuffer);
            renderEngine->returnBuffer(m_IndexBuffer);
            m_VertexBuffer = nullptr;
            m_IndexBuffer = nullptr;
        }
    }

    void VertexBuffer_DirectX12::render(const RenderOptions* renderOptions, Material* material)
    {
        const RenderOptions_DirectX12* renderOptionsDirectX = reinterpret_cast<const RenderOptions_DirectX12*>(renderOptions);
        Material_DirectX12* materialDirectX = dynamic_cast<Material_DirectX12*>(material);
        if ((materialDirectX == nullptr) || !materialDirectX->bindMaterial(renderOptionsDirectX, this))
        {
            return;
        }

        ID3D12GraphicsCommandList2* commandList = renderOptionsDirectX->renderCommandList->get();

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
        vertexBufferView.BufferLocation = m_VertexBuffer->get()->GetGPUVirtualAddress();
        vertexBufferView.SizeInBytes = m_VertexBuffer->getSize();
        vertexBufferView.StrideInBytes = m_CachedVertexSize;
        commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
        if (m_IndexBuffer != nullptr)
        {
            D3D12_INDEX_BUFFER_VIEW indexBufferView{};
            indexBufferView.BufferLocation = m_IndexBuffer->get()->GetGPUVirtualAddress();
            indexBufferView.SizeInBytes = m_IndexBuffer->getSize();
            indexBufferView.Format = DXGI_FORMAT_R32_UINT;
            commandList->IASetIndexBuffer(&indexBufferView);

            commandList->DrawIndexedInstanced(m_RenderElementsCount, 1, 0, 0, 0);
        }
        else
        {
            commandList->DrawInstanced(m_RenderElementsCount, 1, 0, 0);
        }

        materialDirectX->unbindMaterial(renderOptionsDirectX, this);
    }
}

#endif

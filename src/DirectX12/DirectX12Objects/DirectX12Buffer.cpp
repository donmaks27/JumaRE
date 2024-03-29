﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX12)

#include "DirectX12Buffer.h"

#include "../RenderEngine_DirectX12.h"

namespace JumaRenderEngine
{
    DirectX12Buffer::~DirectX12Buffer()
    {
        clearDirectX();
    }

    bool DirectX12Buffer::initStaging(const uint32 size)
    {
        if (isValid())
        {
            JUTILS_LOG(error, JSTR("DirectX12 buffer already initialized"));
            return false;
        }
        if (size == 0)
        {
            JUTILS_LOG(error, JSTR("Size param is zero"));
            return false;
        }

        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();

        D3D12_RESOURCE_DESC resourceDescription{};
        resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDescription.Alignment = 0;
        resourceDescription.Width = size;
        resourceDescription.Height = 1;
        resourceDescription.DepthOrArraySize = 1;
        resourceDescription.MipLevels = 1;
        resourceDescription.Format = DXGI_FORMAT_UNKNOWN;
        resourceDescription.SampleDesc.Count = 1;
        resourceDescription.SampleDesc.Quality = 0;
        resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;
        D3D12MA::ALLOCATION_DESC allocationDescription{};
        allocationDescription.HeapType = D3D12_HEAP_TYPE_UPLOAD;
        const HRESULT result = renderEngine->getResourceAllocator()->CreateResource(
            &allocationDescription, &resourceDescription, 
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, 
            &m_Allocation, IID_PPV_ARGS(&m_Buffer)
        );
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create staging DirectX12 buffer"));
            return false;
        }

        m_BufferSize = size;
        m_BufferState = D3D12_RESOURCE_STATE_GENERIC_READ;
        m_Mapable = true;
        markAsInitialized();
        return true;
    }
    bool DirectX12Buffer::initGPU(const uint32 size, const void* data, const D3D12_RESOURCE_STATES bufferState)
    {
        if (isValid())
        {
            JUTILS_LOG(error, JSTR("DirectX12 buffer already initialized"));
            return false;
        }
        if (size == 0)
        {
            JUTILS_LOG(error, JSTR("Size param is zero"));
            return false;
        }

        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();

        D3D12_RESOURCE_DESC resourceDescription{};
        resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDescription.Alignment = 0;
        resourceDescription.Width = size;
        resourceDescription.Height = 1;
        resourceDescription.DepthOrArraySize = 1;
        resourceDescription.MipLevels = 1;
        resourceDescription.Format = DXGI_FORMAT_UNKNOWN;
        resourceDescription.SampleDesc.Count = 1;
        resourceDescription.SampleDesc.Quality = 0;
        resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;
        D3D12MA::ALLOCATION_DESC allocationDescription{};
        allocationDescription.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        const HRESULT result = renderEngine->getResourceAllocator()->CreateResource(
            &allocationDescription, &resourceDescription, 
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, 
            &m_Allocation, IID_PPV_ARGS(&m_Buffer)
        );
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX12 GPU buffer"));
            return false;
        }

        m_BufferSize = size;
        m_BufferState = D3D12_RESOURCE_STATE_COPY_DEST;
        m_Mapable = false;

        DirectX12Buffer* stagingBuffer = renderEngine->getBuffer();
        if (!stagingBuffer->initStaging(size) || 
            !stagingBuffer->setData(data, size, 0, true) || 
            !stagingBuffer->copyData(nullptr, this, bufferState, true))
        {
            JUTILS_LOG(error, JSTR("Failed to copy data to GPU buffer"));
            renderEngine->returnBuffer(stagingBuffer);
            clearDirectX();
            return false;
        }
        renderEngine->returnBuffer(stagingBuffer);

        markAsInitialized();
        return true;
    }
    bool DirectX12Buffer::initAccessedGPU(const uint32 size, const D3D12_RESOURCE_STATES bufferState)
    {
        if (isValid())
        {
            JUTILS_LOG(error, JSTR("DirectX12 buffer already initialized"));
            return false;
        }
        if (size == 0)
        {
            JUTILS_LOG(error, JSTR("Size param is zero"));
            return false;
        }

        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();

        D3D12_RESOURCE_DESC resourceDescription{};
        resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDescription.Alignment = 0;
        resourceDescription.Width = size;
        resourceDescription.Height = 1;
        resourceDescription.DepthOrArraySize = 1;
        resourceDescription.MipLevels = 1;
        resourceDescription.Format = DXGI_FORMAT_UNKNOWN;
        resourceDescription.SampleDesc.Count = 1;
        resourceDescription.SampleDesc.Quality = 0;
        resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE;
        D3D12MA::ALLOCATION_DESC allocationDescription{};
        allocationDescription.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        const HRESULT result = renderEngine->getResourceAllocator()->CreateResource(
            &allocationDescription, &resourceDescription, 
            bufferState, nullptr, 
            &m_Allocation, IID_PPV_ARGS(&m_Buffer)
        );
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX12 GPU buffer"));
            return false;
        }

        DirectX12Buffer* stagingBuffer = renderEngine->getBuffer();
        if (!stagingBuffer->initStaging(size))
        {
            JUTILS_LOG(error, JSTR("Failed to create staging buffer for GPU buffer"));
            renderEngine->returnBuffer(stagingBuffer);
            clearDirectX();
            return false;
        }
        m_StagingBuffer = stagingBuffer;

        m_BufferSize = size;
        m_BufferState = bufferState;
        m_Mapable = false;
        markAsInitialized();
        return true;
    }

    void DirectX12Buffer::clearDirectX()
    {
        m_MappedData = nullptr;
        m_Mapable = false;
        if (m_StagingBuffer != nullptr)
        {
            getRenderEngine<RenderEngine_DirectX12>()->returnBuffer(m_StagingBuffer);
            m_StagingBuffer = nullptr;
        }
        if (m_Buffer != nullptr)
        {
            m_Buffer->Release();
            m_Allocation->Release();
            m_Buffer = nullptr;
            m_Allocation = nullptr;
        }
        m_BufferSize = 0;
    }

    bool DirectX12Buffer::initMappedData()
    {
        if (!isValid())
        {
            JUTILS_LOG(error, JSTR("DirectX12 buffer not initialized"));
            return false;
        }

        if (m_StagingBuffer != nullptr)
        {
            if (!m_StagingBuffer->initMappedData())
            {
                JUTILS_LOG(error, JSTR("Failed to map data for staging buffer"));
                return false;
            }
            return true;
        }
        if (!m_Mapable)
        {
            JUTILS_LOG(warning, JSTR("Unable to update buffer data"));
            return false;
        }

        if (m_MappedData == nullptr)
        {
            void* data;
            const HRESULT result = m_Buffer->Map(0, nullptr, &data);
            if (FAILED(result))
            {
                JUTILS_ERROR_LOG(result, JSTR("Failed to map DirectX12 buffer"));
                return false;
            }
            m_MappedData = data;
        }
        return true;
    }
    void* DirectX12Buffer::getMappedData(const uint32 offset) const
    {
        if (m_MappedData != nullptr)
        {
            return static_cast<uint8*>(m_MappedData) + offset;
        }
        return m_StagingBuffer != nullptr ? m_StagingBuffer->getMappedData(offset) : nullptr;
    }
    bool DirectX12Buffer::setMappedData(const void* data, const uint32 size, const uint32 offset) const
    {
        if ((data == nullptr) || (size == 0) || ((offset + size) > m_BufferSize))
        {
            JUTILS_LOG(error, JSTR("Invalid input params"));
            return false;
        }

        void* mappedData = getMappedData(offset);
        if (data == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to get mapped data"));
            return false;
        }
        std::memcpy(mappedData, data, size);
        return true;
    }
    bool DirectX12Buffer::flushMappedData(DirectX12CommandList* commandList, const bool waitForFinish)
    {
        if (!isValid())
        {
            JUTILS_LOG(error, JSTR("DirectX12 buffer not initialized"));
            return false;
        }

        if (m_StagingBuffer != nullptr)
        {
            if (!m_StagingBuffer->flushMappedData(nullptr, false) || !m_StagingBuffer->copyData(commandList, this, m_BufferState, waitForFinish))
            {
                JUTILS_LOG(error, JSTR("Failed to flush data from staging DirectX12 buffer"));
                return false;
            }
            return true;
        }

        if (m_MappedData != nullptr)
        {
            m_Buffer->Unmap(0, nullptr);
            m_MappedData = nullptr;
        }
        return true;
    }

    bool DirectX12Buffer::setData(const void* data, const uint32 size, const uint32 offset, const bool waitForFinish)
    {
        if (!isValid())
        {
            JUTILS_LOG(error, JSTR("DirectX12 buffer not initialized"));
            return false;
        }
        if ((data == nullptr) || (size == 0) || ((offset + size) > m_BufferSize))
        {
            JUTILS_LOG(error, JSTR("Invalid input params"));
            return false;
        }
        if (m_StagingBuffer != nullptr)
        {
            if (!m_StagingBuffer->setDataInternal(data, size, offset) || !m_StagingBuffer->copyData(nullptr, this, m_BufferState, waitForFinish))
            {
                JUTILS_LOG(error, JSTR("Failed to copy data from staging buffer"));
                return false;
            }
            return true;
        }
        return setDataInternal(data, size, offset);
    }
    bool DirectX12Buffer::setDataInternal(const void* data, const uint32 size, const uint32 offset)
    {
        if (!m_Mapable)
        {
            JUTILS_LOG(warning, JSTR("Unable to update buffer data"));
            return false;
        }

        void* mappedData;
        HRESULT result = m_Buffer->Map(0, nullptr, &mappedData);
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to map DirectX12 buffer"));
            return false;
        }
        std::memcpy(static_cast<uint8*>(mappedData) + offset, data, size);
        m_Buffer->Unmap(0, nullptr);
        return true;
    }
    bool DirectX12Buffer::copyData(DirectX12CommandList* commandListObject, DirectX12Buffer* destinationBuffer, 
        const D3D12_RESOURCE_STATES finalState, const bool waitForFinish) const
    {
        const bool shouldCreateCommandList = (commandListObject == nullptr);
        if (shouldCreateCommandList)
        {
            const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
            DirectX12CommandQueue* commandQueue = renderEngine->getCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
            commandListObject = commandQueue->getCommandList();
        }

        commandListObject->changeBufferState(destinationBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
        commandListObject->applyStateChanges();
        commandListObject->get()->CopyResource(destinationBuffer->get(), m_Buffer);
        commandListObject->changeBufferState(destinationBuffer, finalState);
        commandListObject->applyStateChanges();

        if (shouldCreateCommandList)
        {
            commandListObject->execute();
            if (waitForFinish)
            {
                commandListObject->waitForFinish();
            }
            commandListObject->markUnused();
        }
        return true;
    }
}

#endif

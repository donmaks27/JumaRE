﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "../../../include/JumaRE/core.h"

#include <d3d12.h>
#include <jutils/jmap.h>

namespace JumaRenderEngine
{
    class DirectX12Buffer;
    class DirectX12Texture;
    class DirectX12CommandQueue;

    class DirectX12CommandList
    {
        friend DirectX12CommandQueue;

    public:
        DirectX12CommandList() = default;
        ~DirectX12CommandList();

        ID3D12GraphicsCommandList2* get() const { return m_CommandList; }

        void execute(bool signalFence = true);
        void signal();
        void waitForFinish();
        void markUnused();
        
        bool isValidForReuse() const;
        void reset();

        void changeTextureState(DirectX12Texture* texture, D3D12_RESOURCE_STATES state);
        void changeTextureState(DirectX12Texture* texture, D3D12_RESOURCE_STATES state, uint8 firstMipLevelIndex, uint8 mipLevelsCount = 1);
        void updateTextureState(DirectX12Texture* texture, D3D12_RESOURCE_STATES state);
        void updateTextureState(DirectX12Texture* texture, D3D12_RESOURCE_STATES state, uint8 firstMipLevelIndex, uint8 mipLevelsCount = 1);

    	void changeBufferState(DirectX12Buffer* buffer, D3D12_RESOURCE_STATES state);

    	void applyStateChanges();

    private:

        DirectX12CommandQueue* m_ParentCommandQueue = nullptr;

        ID3D12CommandAllocator* m_CommandAllocator = nullptr;
        ID3D12GraphicsCommandList2* m_CommandList = nullptr;
        uint64 m_FenceValue = 0;

        jarray<D3D12_RESOURCE_BARRIER> m_ResourceBarriers;
        jmap<DirectX12Texture*, jarray<D3D12_RESOURCE_STATES>> m_TextureStates;
        jmap<DirectX12Buffer*, D3D12_RESOURCE_STATES> m_BufferStates;

        bool m_Executed = false;


        bool init(DirectX12CommandQueue* commandQueue);

        void clearDirectX();
    };
}

#endif

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "../../include/JumaRE/RenderEngine.h"

#include "D3D12MemAlloc.h"

#include "Material_DirectX12.h"
#include "RenderTarget_DirectX12.h"
#include "Shader_DirectX12.h"
#include "Texture_DirectX12.h"
#include "VertexBuffer_DirectX12.h"
#include "DirectX12Objects/DirectX12Buffer.h"
#include "DirectX12Objects/DirectX12CommandQueue.h"
#include "DirectX12Objects/DirectX12Texture.h"
#include "../utils/RenderEngineObjectsPool.h"
#include "../../include/JumaRE/texture/TextureSamplerType.h"

namespace JumaRenderEngine
{
    class DirectX12MipGenerator;

    class RenderEngine_DirectX12 final : public RenderEngine
    {
        using Super = RenderEngine;

    public:
        RenderEngine_DirectX12() = default;
        virtual ~RenderEngine_DirectX12() override;

        virtual RenderAPI getRenderAPI() const override { return RenderAPI::DirectX12; }

        virtual math::vector2 getScreenCoordinateModifier() const override { return { 1.0f, -1.0f }; }

        ID3D12Device2* getDevice() const { return m_Device; }
        D3D12MA::Allocator* getResourceAllocator() const { return m_ResourceAllocator; }
        DirectX12CommandQueue* getCommandQueue(const D3D12_COMMAND_LIST_TYPE queueType) const { return m_CommandQueues.find(queueType); }
        
        ID3D12DescriptorHeap* createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 size, bool shaderVisible) const;
        template<D3D12_DESCRIPTOR_HEAP_TYPE Type>
        uint8 getDescriptorSize() const { return getCachedDescriptorSize<Type>(); }
        template<D3D12_DESCRIPTOR_HEAP_TYPE Type>
        D3D12_CPU_DESCRIPTOR_HANDLE getDescriptorCPU(ID3D12DescriptorHeap* descriptorHeap, const uint32 descriptorIndex) const
        {
            return { descriptorHeap != nullptr ? descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + descriptorIndex * getDescriptorSize<Type>() : 0 };
        }

        D3D12_CPU_DESCRIPTOR_HANDLE getSamplerDescription(const TextureSamplerType& samplerType) const
        {
            return getDescriptorCPU<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER>(m_SamplersDescriptorHeap, GetTextureSamplerTypeID(samplerType));
        }

        DirectX12MipGenerator* getMipGenerator() const { return m_TextureMipGenerator; }

        DirectX12Buffer* getBuffer() { return m_BuffersPool.getObject(this); }
        DirectX12Texture* getDirectXTexture() { return m_DirectXTexturesPool.getObject(this); }
        void returnBuffer(DirectX12Buffer* buffer) { m_BuffersPool.returnObject(buffer); }
        void returnDirectXTexture(DirectX12Texture* texture) { m_DirectXTexturesPool.returnObject(texture); }

    protected:

        virtual bool initInternal(const WindowCreateInfo& mainWindowInfo) override;
        virtual void clearInternal() override;

        virtual WindowController* createWindowController() override;
        virtual RenderPipeline* createRenderPipelineInternal() override;
        virtual RenderTarget* allocateRenderTarget() override { return m_RenderTargetsPool.getObject(this); }
        virtual VertexBuffer* allocateVertexBuffer() override { return m_VertexBuffersPool.getObject(this); }
        virtual Shader* allocateShader() override { return m_ShadersPool.getObject(this); }
        virtual Material* allocateMaterial() override { return m_MaterialsPool.getObject(this); }
        virtual Texture* allocateTexture() override { return m_TexturesPool.getObject(this); }

        virtual void deallocateRenderTarget(RenderTarget* renderTarget) override { m_RenderTargetsPool.returnObject(renderTarget); }
        virtual void deallocateVertexBuffer(VertexBuffer* vertexBuffer) override { m_VertexBuffersPool.returnObject(vertexBuffer); }
        virtual void deallocateShader(Shader* shader) override { m_ShadersPool.returnObject(shader); }
        virtual void deallocateMaterial(Material* material) override { m_MaterialsPool.returnObject(material); }
        virtual void deallocateTexture(Texture* texture) override { m_TexturesPool.returnObject(texture); }

    private:

        ID3D12Device2* m_Device = nullptr;
        D3D12MA::Allocator* m_ResourceAllocator = nullptr;
        mutable jmap<D3D12_COMMAND_LIST_TYPE, DirectX12CommandQueue> m_CommandQueues;

        uint8 m_CachedDescriptorSize_RTV = 0;
        uint8 m_CachedDescriptorSize_DSV = 0;
        uint8 m_CachedDescriptorSize_SRV = 0;
        uint8 m_CachedDescriptorSize_Sampler = 0;

        ID3D12DescriptorHeap* m_SamplersDescriptorHeap = nullptr;

        DirectX12MipGenerator* m_TextureMipGenerator = nullptr;

        RenderEngineObjectsPool<RenderTarget, RenderTarget_DirectX12> m_RenderTargetsPool;
        RenderEngineObjectsPool<VertexBuffer, VertexBuffer_DirectX12> m_VertexBuffersPool;
        RenderEngineObjectsPool<Shader, Shader_DirectX12> m_ShadersPool;
        RenderEngineObjectsPool<Material, Material_DirectX12> m_MaterialsPool;
        RenderEngineObjectsPool<Texture, Texture_DirectX12> m_TexturesPool;

        RenderEngineObjectsPool<DirectX12Buffer> m_BuffersPool;
        RenderEngineObjectsPool<DirectX12Texture> m_DirectXTexturesPool;


        bool createDirectXDevice();
        bool createCommandQueues();
        bool createOtherObjects();

        void clearDirectX();

        template<D3D12_DESCRIPTOR_HEAP_TYPE Type>
        uint8 getCachedDescriptorSize() const { return 0; }
        template<>
        uint8 getCachedDescriptorSize<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>() const { return m_CachedDescriptorSize_RTV; }
        template<>
        uint8 getCachedDescriptorSize<D3D12_DESCRIPTOR_HEAP_TYPE_DSV>() const { return m_CachedDescriptorSize_DSV; }
        template<>
        uint8 getCachedDescriptorSize<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>() const { return m_CachedDescriptorSize_SRV; }
        template<>
        uint8 getCachedDescriptorSize<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER>() const { return m_CachedDescriptorSize_Sampler; }
    };
}

#endif

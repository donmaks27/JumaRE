﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX12)

#include "RenderTarget_DirectX12.h"

#include "RenderEngine_DirectX12.h"
#include "RenderOptions_DirectX12.h"
#include "DirectX12Objects/DirectX12MipGenerator.h"
#include "DirectX12Objects/DirectX12Swapchain.h"
#include "DirectX12Objects/DirectX12Texture.h"
#include "window/WindowController_DirectX12.h"
#include "../DirectX/TextureFormat_DirectX.h"

namespace JumaRenderEngine
{
    RenderTarget_DirectX12::~RenderTarget_DirectX12()
    {
        clearDirectX();
    }

    bool RenderTarget_DirectX12::initInternal()
    {
        if (!Super::initInternal())
        {
            return false;
        }
        if (isWindowRenderTarget())
        {
            if (!initWindowRenderTarget())
            {
                JUTILS_LOG(error, JSTR("Failed to init DirectX12 window render target"));
                return false;
            }
        }
        else if (!initRenderTarget())
        {
            JUTILS_LOG(error, JSTR("Failed to init DirectX12 render target"));
            return false;
        }
        if (!initMipGeneratorTarget())
        {
            JUTILS_LOG(error, JSTR("Failed to init mip generator target"));
            clearDirectX();
            return false;
        }
        return true;
    }
    bool RenderTarget_DirectX12::initWindowRenderTarget()
    {
        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        const window_id windowID = getWindowID();
        const WindowData_DirectX12* windowData = renderEngine->getWindowController()->findWindowData<WindowData_DirectX12>(windowID);
        const DirectX12Swapchain* swapchain = windowData != nullptr ? windowData->swapchain : nullptr;
        if (swapchain == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to find swapchain for window {}"), windowID);
            return false;
        }

        ID3D12Device2* device = renderEngine->getDevice();
        const TextureSamples samples = getSampleCount();
        const uint8 samplesCount = GetTextureSamplesNumber(samples);
        const math::uvector2 size = windowData->size;
        const bool depthEnabled = isDepthEnabled();

        const uint8 buffersCount = swapchain->getBuffersCount();
        jarray<DirectX12Texture*> buffers(buffersCount, nullptr);
        for (uint8 bufferIndex = 0; bufferIndex < buffersCount; bufferIndex++)
        {
            buffers[bufferIndex] = swapchain->getBuffer(bufferIndex);
        }

        DirectX12Texture* renderTexture = nullptr;
        ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
        if (samplesCount > 1)
        {
            renderTexture = renderEngine->getDirectXTexture();
            renderTexture->initColor(
                size, samplesCount, GetDirectXFormatByTextureFormat(getColorFormat()), 1, D3D12_RESOURCE_STATE_RENDER_TARGET, 
                D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
            );
            if (!renderTexture->isValid())
            {
                JUTILS_LOG(error, JSTR("Failed to create render texture"));
                renderEngine->returnDirectXTexture(renderTexture);
                return false;
            }

            rtvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
            if (rtvDescriptorHeap == nullptr)
            {
                JUTILS_LOG(error, JSTR("Failed to create RTV descriptor for render texture"));
                renderEngine->returnDirectXTexture(renderTexture);
                return false;
            }
            device->CreateRenderTargetView(renderTexture->getResource(), nullptr, rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }
        else
        {
            rtvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, buffersCount, false);
            if (rtvDescriptorHeap == nullptr)
            {
                JUTILS_LOG(error, JSTR("Failed to create RTV descriptor for render texture"));
                return false;
            }
            for (uint8 bufferIndex = 0; bufferIndex < buffersCount; bufferIndex++)
            {
                device->CreateRenderTargetView(
                    buffers[bufferIndex]->getResource(), nullptr, 
                    renderEngine->getDescriptorCPU<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>(rtvDescriptorHeap, bufferIndex)
                );
            }
        }

        DirectX12Texture* depthTexture = nullptr;
        ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
        if (depthEnabled)
        {
            depthTexture = renderEngine->getDirectXTexture();
            depthTexture->initDepth(
                size, samplesCount, GetDirectXFormatByTextureFormat(TextureFormat::DEPTH24_STENCIL8), D3D12_RESOURCE_STATE_DEPTH_WRITE, 
                D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
            );
            if (!depthTexture->isValid())
            {
                JUTILS_LOG(error, JSTR("Failed to create depth texture"));
                renderEngine->returnDirectXTexture(depthTexture);
                rtvDescriptorHeap->Release();
                renderEngine->returnDirectXTexture(renderTexture);
                return false;
            }

            dsvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
            if (dsvDescriptorHeap == nullptr)
            {
                JUTILS_LOG(error, JSTR("Failed to create descriptor heap for DSV"));
                renderEngine->returnDirectXTexture(depthTexture);
                rtvDescriptorHeap->Release();
                renderEngine->returnDirectXTexture(renderTexture);
                return false;
            }
            device->CreateDepthStencilView(depthTexture->getResource(), nullptr, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }

        m_ColorTexture = renderTexture;
        m_ResultTextures = buffers;
        m_DescriptorHeapRTV = rtvDescriptorHeap;
        m_DepthTexture = depthTexture;
        m_DescriptorHeapDSV = dsvDescriptorHeap;
        return true;
    }
    bool RenderTarget_DirectX12::initRenderTarget()
    {
        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        ID3D12Device2* device = renderEngine->getDevice();

        const DXGI_FORMAT format = GetDirectXFormatByTextureFormat(getColorFormat());
        const TextureSamples samples = getSampleCount();
        const uint8 samplesCount = GetTextureSamplesNumber(samples);
        const bool shouldResolve = samplesCount > 1;
        const math::uvector2 size = getSize();
        const bool depthEnabled = isDepthEnabled();
        const uint8 colorMipLevelsCount = shouldResolve ? 1 : 0;
        //const uint8 colorMipLevelsCount = 1;
        const uint8 resolveMipLevelsCount = 0;
        //const uint8 resolveMipLevelsCount = 1;

        DirectX12Texture* renderTexture = renderEngine->getDirectXTexture();
        renderTexture->initColor(
            size, samplesCount, format, colorMipLevelsCount, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );
        if (!renderTexture->isValid())
        {
            JUTILS_LOG(error, JSTR("Failed to create render texture"));
            renderEngine->returnDirectXTexture(renderTexture);
            return false;
        }

        DirectX12Texture* depthTexture = nullptr;
        if (depthEnabled)
        {
            depthTexture = renderEngine->getDirectXTexture();
            depthTexture->initDepth(
                size, samplesCount, GetDirectXFormatByTextureFormat(TextureFormat::DEPTH24_STENCIL8), D3D12_RESOURCE_STATE_DEPTH_WRITE, 
                D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
            );
            if (!depthTexture->isValid())
            {
                JUTILS_LOG(error, JSTR("Failed to create depth texture"));
                renderEngine->returnDirectXTexture(depthTexture);
                renderEngine->returnDirectXTexture(renderTexture);
                return false;
            }
        }

        DirectX12Texture* resolveTexture = nullptr;
        if (shouldResolve)
        {
            resolveTexture = renderEngine->getDirectXTexture();
            resolveTexture->initColor(size, 1, format, resolveMipLevelsCount, D3D12_RESOURCE_STATE_RESOLVE_DEST);
            if (!resolveTexture->isValid())
            {
                JUTILS_LOG(error, JSTR("Failed to create resolve texture"));
                renderEngine->returnDirectXTexture(resolveTexture);
                renderEngine->returnDirectXTexture(depthTexture);
                renderEngine->returnDirectXTexture(renderTexture);
                return false;
            }
        }

        ID3D12DescriptorHeap* rtvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
        if (rtvDescriptorHeap == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to create RTV descriptor heap"));
            renderEngine->returnDirectXTexture(resolveTexture);
            renderEngine->returnDirectXTexture(depthTexture);
            renderEngine->returnDirectXTexture(renderTexture);
            return false;
        }
        ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
        if (depthEnabled)
        {
            dsvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
            if (dsvDescriptorHeap == nullptr)
            {
                JUTILS_LOG(error, JSTR("Failed to create descriptor heap for DSV"));
                rtvDescriptorHeap->Release();
                renderEngine->returnDirectXTexture(resolveTexture);
                renderEngine->returnDirectXTexture(depthTexture);
                renderEngine->returnDirectXTexture(renderTexture);
                return false;
            }
        }
        ID3D12DescriptorHeap* srvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, false);
        if (srvDescriptorHeap == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to create descriptor heap for SRV"));
            if (depthEnabled)
            {
                dsvDescriptorHeap->Release();
            }
            rtvDescriptorHeap->Release();
            renderEngine->returnDirectXTexture(resolveTexture);
            renderEngine->returnDirectXTexture(depthTexture);
            renderEngine->returnDirectXTexture(renderTexture);
            return false;
        }

        device->CreateRenderTargetView(renderTexture->getResource(), nullptr, rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        if (depthEnabled)
        {
            device->CreateDepthStencilView(depthTexture->getResource(), nullptr, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }
        device->CreateShaderResourceView(resolveTexture != nullptr ? resolveTexture->getResource() : renderTexture->getResource(), nullptr, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        m_ColorTexture = renderTexture;
        if (resolveTexture != nullptr)
        {
            m_ResultTextures = { resolveTexture };
        }
        m_DescriptorHeapRTV = rtvDescriptorHeap;
        m_DescriptorHeapSRV = srvDescriptorHeap;
        m_DepthTexture = depthTexture;
        m_DescriptorHeapDSV = dsvDescriptorHeap;
        return true;
    }

    void RenderTarget_DirectX12::clearAssetInternal()
    {
        clearDirectX();
        Super::clearAssetInternal();
    }
    void RenderTarget_DirectX12::clearDirectX()
    {
        clearRenderTarget();
    }
    void RenderTarget_DirectX12::clearRenderTarget()
    {
        clearMipGeneratorTarget();

        if (m_DescriptorHeapDSV != nullptr)
        {
            m_DescriptorHeapDSV->Release();
            m_DescriptorHeapDSV = nullptr;
        }
        if (m_DescriptorHeapSRV != nullptr)
        {
            m_DescriptorHeapSRV->Release();
            m_DescriptorHeapSRV = nullptr;
        }
        if (m_DescriptorHeapRTV != nullptr)
        {
            m_DescriptorHeapRTV->Release();
            m_DescriptorHeapRTV = nullptr;
        }

        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        if (m_ColorTexture != nullptr)
        {
            renderEngine->returnDirectXTexture(m_ColorTexture);
            m_ColorTexture = nullptr;
        }
        if (m_DepthTexture != nullptr)
        {
            renderEngine->returnDirectXTexture(m_DepthTexture);
            m_DepthTexture = nullptr;
        }
        if (!isWindowRenderTarget())
        {
            for (const auto& texture : m_ResultTextures)
            {
                renderEngine->returnDirectXTexture(texture);
            }
        }
        m_ResultTextures.clear();
    }

    bool RenderTarget_DirectX12::recreateRenderTarget()
    {
        clearRenderTarget();
        return isWindowRenderTarget() ? initWindowRenderTarget() : initRenderTarget();
    }

    bool RenderTarget_DirectX12::onStartRender(RenderOptions* renderOptions)
    {
        if (!Super::onStartRender(renderOptions))
        {
            return false;
        }

        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();

        math::uvector2 size;
        DirectX12Texture* renderTexture = nullptr;
        uint8 rtvIndex = 0;
        if (isWindowRenderTarget())
        {
            const WindowData_DirectX12* windowData = renderEngine->getWindowController()->findWindowData<WindowData_DirectX12>(getWindowID());
            if (windowData == nullptr)
            {
                return false;
            }

            size = windowData->size;
            if (getSampleCount() == TextureSamples::X1)
            {
                rtvIndex = windowData->swapchain->getCurrentBufferIndex();
                renderTexture = m_ResultTextures[rtvIndex];
            }
            else
            {
                rtvIndex = 0;
                renderTexture = m_ColorTexture;
            }
        }
        else
        {
            size = getSize();
            rtvIndex = 0;
            renderTexture = m_ColorTexture;
        }
        if (renderTexture == nullptr)
        {
            JUTILS_LOG(error, JSTR("Invalid render target"));
            return false;
        }

        DirectX12CommandList* commandListObject = reinterpret_cast<RenderOptions_DirectX12*>(renderOptions)->renderCommandList;
        ID3D12GraphicsCommandList2* commandList = commandListObject->get();

        commandListObject->changeTextureState(renderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandListObject->applyStateChanges();

        const D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor = renderEngine->getDescriptorCPU<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>(m_DescriptorHeapRTV, rtvIndex);
        static constexpr FLOAT clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        commandList->ClearRenderTargetView(rtvDescriptor, clearColor, 0, nullptr);
        if (m_DescriptorHeapDSV != nullptr)
        {
            const D3D12_CPU_DESCRIPTOR_HANDLE dsvDescriptor = m_DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart();
            commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
            commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
        }
        else
        {
            commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, nullptr);
        }

        const D3D12_VIEWPORT viewport = {
            0.0f, 0.0f, static_cast<FLOAT>(size.x), static_cast<FLOAT>(size.y), 0.0f, 1.0f
        };
        constexpr D3D12_RECT scissors = { 0, 0, LONG_MAX, LONG_MAX };
        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissors);
        return true;
    }

    void RenderTarget_DirectX12::onFinishRender(RenderOptions* renderOptions)
    {
        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        const bool shouldResolve = getSampleCount() != TextureSamples::X1;

        DirectX12Texture* renderTexture = nullptr;
        DirectX12Texture* resolveTexture = nullptr;
        if (isWindowRenderTarget())
        {
            const WindowData_DirectX12* windowData = renderEngine->getWindowController()->findWindowData<WindowData_DirectX12>(getWindowID());
            if (!shouldResolve)
            {
                renderTexture = m_ResultTextures[windowData->swapchain->getCurrentBufferIndex()];
            }
            else
            {
                renderTexture = m_ColorTexture;
                resolveTexture = m_ResultTextures[windowData->swapchain->getCurrentBufferIndex()];
            }
        }
        else
        {
            renderTexture = m_ColorTexture;
            if (shouldResolve)
            {
                resolveTexture = m_ResultTextures[0];
            }
        }

        DirectX12CommandList* commandListObject = reinterpret_cast<RenderOptions_DirectX12*>(renderOptions)->renderCommandList;
        ID3D12GraphicsCommandList2* commandList = commandListObject->get();

        if (!shouldResolve)
        {
            if (!isWindowRenderTarget())
            {
                renderEngine->getMipGenerator()->generateMips(commandListObject, this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            }
            else
            {
                commandListObject->changeTextureState(renderTexture, D3D12_RESOURCE_STATE_PRESENT);
            }
            commandListObject->applyStateChanges();
        }
        else
        {
            commandListObject->changeTextureState(renderTexture, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
            commandListObject->changeTextureState(resolveTexture, D3D12_RESOURCE_STATE_RESOLVE_DEST);
            commandListObject->applyStateChanges();

            commandList->ResolveSubresource(
                resolveTexture->getResource(), 0, renderTexture->getResource(), 0, 
                GetDirectXFormatByTextureFormat(getColorFormat())
            );

            commandListObject->changeTextureState(renderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
            if (!isWindowRenderTarget())
            {
                renderEngine->getMipGenerator()->generateMips(commandListObject, this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            }
            else
            {
                commandListObject->changeTextureState(resolveTexture, D3D12_RESOURCE_STATE_PRESENT);
            }
            commandListObject->applyStateChanges();
        }

        Super::onFinishRender(renderOptions);
    }
}

#endif

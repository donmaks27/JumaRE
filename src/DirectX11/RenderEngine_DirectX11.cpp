﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX11)

#include "RenderEngine_DirectX11.h"

#include <d3d11.h>

#include "window/WindowControllerImpl_DirectX11.h"
#include "window/WindowController_DirectX11.h"

namespace JumaRenderEngine
{
    RenderEngine_DirectX11::~RenderEngine_DirectX11()
    {
        clearDirectX();
    }

    bool RenderEngine_DirectX11::initInternal(const WindowCreateInfo& mainWindowInfo)
    {
        if (!Super::initInternal(mainWindowInfo))
        {
            return false;
        }
        if (!createDirectXDevice())
        {
            JUTILS_LOG(error, JSTR("Failed to create device"));
            return false;
        }
        if (!getWindowController<WindowController_DirectX11>()->createWindowSwapchains())
        {
            JUTILS_LOG(error, JSTR("Failed to create DirectX11 swapchains"));
            return false;
        }
        return true;
    }
    bool RenderEngine_DirectX11::createDirectXDevice()
    {
#ifdef JDEBUG
        constexpr UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
        constexpr UINT createDeviceFlags = 0;
#endif
        constexpr D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        const HRESULT result = D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 
            createDeviceFlags, featureLevels, 4, D3D11_SDK_VERSION, 
            &m_Device, nullptr, &m_DeviceContext
        );
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX11 device"));
            return false;
        }
        return true;
    }

    void RenderEngine_DirectX11::clearInternal()
    {
        clearDirectX();
        Super::clearInternal();
    }
    void RenderEngine_DirectX11::clearDirectX()
    {
        clearAssets();
        m_MaterialsPool.clear();
        m_TexturesPool.clear();
        m_ShadersPool.clear();
        m_VertexBuffersPool.clear();
        m_RenderTargetsPool.clear();
        {
            WindowController_DirectX11* windowController = getWindowController<WindowController_DirectX11>();
            if (windowController != nullptr)
            {
                windowController->destroyWindowSwapchains();
            }
        }

        for (const auto& samplerState : m_TextureSamplers.values())
        {
            samplerState->Release();
        }
        for (const auto& blendState : m_BlendStates.values())
        {
            blendState->Release();
        }
        for (const auto& rasterizerState : m_RasterizerStates.values())
        {
            rasterizerState->Release();
        }
        for (const auto& depthStencilState : m_DepthStencilStates.values())
        {
            depthStencilState->Release();
        }
        m_TextureSamplers.clear();
        m_BlendStates.clear();
        m_DepthStencilStates.clear();
        m_RasterizerStates.clear();

        if (m_DeviceContext != nullptr)
        {
            m_DeviceContext->Release();
            m_DeviceContext = nullptr;
        }
        if (m_Device != nullptr)
        {
            m_Device->Release();
            m_Device = nullptr;
        }
    }

    WindowController* RenderEngine_DirectX11::createWindowController()
    {
        return CreateWindowController_DirectX11();
    }

    ID3D11DepthStencilState* RenderEngine_DirectX11::getDepthStencilState(const DepthStencilState& description)
    {
        ID3D11DepthStencilState** statePtr = m_DepthStencilStates.find(description);
        if (statePtr != nullptr)
        {
            return *statePtr;
        }

        D3D11_DEPTH_STENCIL_DESC depthStateDescription{};
        depthStateDescription.DepthEnable = description.depthEnabled ? TRUE : FALSE;
        depthStateDescription.DepthWriteMask = description.depthEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        depthStateDescription.DepthFunc = D3D11_COMPARISON_LESS;
        depthStateDescription.StencilEnable = description.stencilEnabled ? TRUE : FALSE;
        depthStateDescription.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        depthStateDescription.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        depthStateDescription.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStateDescription.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        depthStateDescription.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStateDescription.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        depthStateDescription.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStateDescription.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        depthStateDescription.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStateDescription.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        ID3D11DepthStencilState* depthStencilState = nullptr;
        const HRESULT result = m_Device->CreateDepthStencilState(&depthStateDescription, &depthStencilState);
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create depth stencil state"));
            return nullptr;
        }
        return m_DepthStencilStates.add(description, depthStencilState);
    }
    ID3D11RasterizerState* RenderEngine_DirectX11::getRasterizerState(const RasterizationState& description)
    {
        ID3D11RasterizerState** statePtr = m_RasterizerStates.find(description);
        if (statePtr != nullptr)
        {
            return *statePtr;
        }

        D3D11_RASTERIZER_DESC rasterizerDescription{};
        rasterizerDescription.FillMode = description.wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
        rasterizerDescription.CullMode = description.cullBackFaces ? D3D11_CULL_BACK : D3D11_CULL_FRONT;
        rasterizerDescription.FrontCounterClockwise = TRUE;
        rasterizerDescription.DepthBias = 0;
        rasterizerDescription.DepthBiasClamp = 0.0f;
        rasterizerDescription.SlopeScaledDepthBias = 0.0f;
        rasterizerDescription.DepthClipEnable = TRUE;
        rasterizerDescription.ScissorEnable = FALSE;
        rasterizerDescription.MultisampleEnable = TRUE;
        rasterizerDescription.AntialiasedLineEnable = TRUE;
        ID3D11RasterizerState* rasterizerState;
        const HRESULT result = m_Device->CreateRasterizerState(&rasterizerDescription, &rasterizerState);
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create rasterizer state"));
            return nullptr;
        }
        return m_RasterizerStates.add(description, rasterizerState);
    }
    ID3D11BlendState* RenderEngine_DirectX11::getBlendState(const BlendState& description)
    {
        ID3D11BlendState** statePtr = m_BlendStates.find(description);
        if (statePtr != nullptr)
        {
            return *statePtr;
        }

        D3D11_BLEND_DESC blendDescription{};
        blendDescription.AlphaToCoverageEnable = FALSE;
        blendDescription.IndependentBlendEnable = FALSE;
        blendDescription.RenderTarget[0].BlendEnable = description.blendEnabled ? TRUE : FALSE;
        blendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        blendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
        blendDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        ID3D11BlendState* blendState;
        const HRESULT result = m_Device->CreateBlendState(&blendDescription, &blendState);
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create blend state"));
            return nullptr;
        }
        return m_BlendStates.add(description, blendState);
    }
    ID3D11SamplerState* RenderEngine_DirectX11::getTextureSampler(const TextureSamplerType samplerType)
    {
        ID3D11SamplerState** samplerPtr = m_TextureSamplers.find(samplerType);
        if (samplerPtr != nullptr)
        {
            return *samplerPtr;
        }

        D3D11_SAMPLER_DESC samplerDescription{};
        switch (samplerType.filterType)
        {
        case TextureFilterType::Point:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            samplerDescription.MaxAnisotropy = 1;
            break;
        case TextureFilterType::Bilinear:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            samplerDescription.MaxAnisotropy = 1;
            break;
        case TextureFilterType::Trilinear:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDescription.MaxAnisotropy = 1;
            break;
        case TextureFilterType::Anisotropic2:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDescription.MaxAnisotropy = 2;
            break;
        case TextureFilterType::Anisotropic4:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDescription.MaxAnisotropy = 4;
            break;
        case TextureFilterType::Anisotropic8:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDescription.MaxAnisotropy = 8;
            break;
        case TextureFilterType::Anisotropic16:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDescription.MaxAnisotropy = 16;
            break;
        default: ;
        }
        switch (samplerType.wrapMode)
        {
        case TextureWrapMode::Repeat: 
            samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            break;
        case TextureWrapMode::Mirror: 
            samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
            samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
            samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
            break;
        case TextureWrapMode::Clamp: 
            samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            break;
        default: ;
        }
        samplerDescription.MipLODBias = 0.0f;
        samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDescription.BorderColor[0] = 1.0f;
        samplerDescription.BorderColor[1] = 1.0f;
        samplerDescription.BorderColor[2] = 1.0f;
        samplerDescription.BorderColor[3] = 1.0f;
        samplerDescription.MinLOD = -FLT_MAX;
        samplerDescription.MaxLOD = FLT_MAX;
        ID3D11SamplerState* samplerState = nullptr;
        const HRESULT result = m_Device->CreateSamplerState(&samplerDescription, &samplerState);
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX11 sampler state"));
            return nullptr;
        }

        return m_TextureSamplers[samplerType] = samplerState;
    }
}

#endif

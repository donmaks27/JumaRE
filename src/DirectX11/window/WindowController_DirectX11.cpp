// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX11)

#include "WindowController_DirectX11.h"

#include <dxgi1_3.h>
#include <d3d11.h>

#include "../RenderEngine_DirectX11.h"
#include "../RenderTarget_DirectX11.h"
#include "../../DirectX/DirectXFunctions.h"
#include "../../DirectX/TextureFormat_DirectX.h"

namespace JumaRenderEngine
{
    WindowController_DirectX11::~WindowController_DirectX11()
    {
        clearData_DirectX11();
    }

    void WindowController_DirectX11::clearData_DirectX11()
    {
    }

    void WindowController_DirectX11::clearWindowDataInternal(const window_id windowID, WindowData* windowData)
    {
        Super::clearWindowDataInternal(windowID, windowData);

        WindowData_DirectX11* windowDataDirectX11 = reinterpret_cast<WindowData_DirectX11*>(windowData);
        destroyWindowSwapchain(windowID, windowDataDirectX11);
        windowDataDirectX11->windowHandler = nullptr;
    }

    bool WindowController_DirectX11::createWindowSwapchain(const window_id windowID, WindowData_DirectX11* windowData)
    {
        ID3D11Device* device = getRenderEngine<RenderEngine_DirectX11>()->getDevice();
        if (device == nullptr)
        {
            // DirectX11 device is not created yet
            return true;
        }

        IDXGIFactory3* factory = nullptr;
#if defined(JDEBUG)
        constexpr UINT createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
        constexpr UINT createFactoryFlags = 0;
#endif
        HRESULT result = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory));
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DXGIFactory3"));
            return false;
        }

        constexpr uint8 buffersCount = 2;
        DXGI_SWAP_CHAIN_DESC1 swapchainDescription{};
        swapchainDescription.Width = windowData->size.x;
        swapchainDescription.Height = windowData->size.y;
        swapchainDescription.Format = GetDirectXFormatByTextureFormat(TextureFormat::RGBA8);
        swapchainDescription.Stereo = FALSE;
        swapchainDescription.SampleDesc.Count = 1;
        swapchainDescription.SampleDesc.Quality = 0;
        swapchainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDescription.BufferCount = buffersCount;
        swapchainDescription.Scaling = DXGI_SCALING_STRETCH;
        swapchainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchainDescription.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapchainDescription.Flags = DirectX_IsTearingSupported() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        result = factory->CreateSwapChainForHwnd(device, windowData->windowHandler, &swapchainDescription, nullptr, nullptr, &windowData->swapchain);
        factory->Release();
        if (FAILED(result))
        {
            JUTILS_ERROR_LOG(result, JSTR("Failed to create DirectX11 swapchain"));
            return false;
        }
        return true;
    }
    void WindowController_DirectX11::destroyWindowSwapchain(const window_id windowID, WindowData_DirectX11* windowData)
    {
        if (windowData->swapchain != nullptr)
        {
            windowData->swapchain->Release();
            windowData->swapchain = nullptr;
        }
    }

    bool WindowController_DirectX11::createWindowSwapchains()
    {
        for (const auto& windowID : getWindowIDs())
        {
            if (!createWindowSwapchain(windowID, getWindowData<WindowData_DirectX11>(windowID)))
            {
                JUTILS_LOG(error, JSTR("Failed to create DirectX11 swapchain"));
                return false;
            }
        }
        return true;
    }
    void WindowController_DirectX11::destroyWindowSwapchains()
    {
        for (const auto& windowID : getWindowIDs())
        {
            destroyWindowSwapchain(windowID, getWindowData<WindowData_DirectX11>(windowID));
        }
    }

    void WindowController_DirectX11::onWindowResized(const window_id windowID, WindowData* windowData)
    {
        WindowController::onWindowResized(windowID, windowData);

        if (windowData->minimized)
        {
            return;
        }

        RenderTarget_DirectX11* renderTarget = dynamic_cast<RenderTarget_DirectX11*>(windowData->windowRenderTarget);
        if (renderTarget != nullptr)
        {
            renderTarget->clearRenderTarget();
        }
        IDXGISwapChain1* swapchain = reinterpret_cast<WindowData_DirectX11*>(windowData)->swapchain;
        if (swapchain != nullptr)
        {
            DXGI_SWAP_CHAIN_DESC1 swapchainDescription{};
            swapchain->GetDesc1(&swapchainDescription);
            swapchain->ResizeBuffers(0, windowData->size.x, windowData->size.y, DXGI_FORMAT_UNKNOWN, swapchainDescription.Flags);
        }
    }

    void WindowController_DirectX11::onFinishWindowRender(const window_id windowID)
    {
        const bool vsyncEnabled = true;
        const bool multipleWindows = getWindowIDs().getSize() > 1;
        const bool shouldEnableVsync = (multipleWindows && (getMainWindowID() == windowID)) || (!multipleWindows && vsyncEnabled);
        const uint32 syncInterval = shouldEnableVsync ? 1 : 0;

        const WindowData_DirectX11* windowData = findWindowData<WindowData_DirectX11>(windowID);
        if (windowData->swapchain != nullptr)
        {
            windowData->swapchain->Present(syncInterval, 0);
        }

        Super::onFinishWindowRender(windowID);
    }
}

#endif

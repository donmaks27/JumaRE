// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX12)

#include "WindowController_DirectX12.h"

#include <dxgi1_5.h>

#include "../RenderEngine_DirectX12.h"
#include "../DirectX12Objects/DirectX12Swapchain.h"

namespace JumaRenderEngine
{
    bool IsTearingSupported_DirectX12()
    {
        IDXGIFactory5* factory4 = nullptr;
#if defined(JDEBUG)
        constexpr UINT createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
        constexpr UINT createFactoryFlags = 0;
#endif
        HRESULT result = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory4));
        if (FAILED(result))
        {
            return false;
        }

        IDXGIFactory5* factory5 = nullptr;
        result = factory4->QueryInterface(&factory5);
        factory4->Release();
        if (FAILED(result))
        {
            return false;
        }

        BOOL allowTearing = FALSE;
        factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        factory5->Release();
        return allowTearing == TRUE;
    }

    WindowController_DirectX12::~WindowController_DirectX12()
    {
        clearData_DirectX12();
    }

    bool WindowController_DirectX12::initWindowController()
    {
        if (!Super::initWindowController())
        {
            return false;
        }
        m_TearingSupported = IsTearingSupported_DirectX12();
        return true;
    }

    void WindowController_DirectX12::clearData_DirectX12()
    {
        m_TearingSupported = false;
    }

    void WindowController_DirectX12::destroyWindowInternal(const window_id windowID, WindowData* windowData)
    {
        Super::destroyWindowInternal(windowID, windowData);

        WindowData_DirectX12* windowDataDirectX12 = reinterpret_cast<WindowData_DirectX12*>(windowData);
        destroyWindowSwapchain(windowID, windowDataDirectX12);
        windowDataDirectX12->windowHandler = nullptr;
    }

    bool WindowController_DirectX12::createWindowSwapchain(const window_id windowID, WindowData_DirectX12* windowData)
    {
        if (windowData->swapchain != nullptr)
        {
            return true;
        }

        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        if (renderEngine->getDevice() == nullptr)
        {
            return true;
        }

        DirectX12Swapchain* swapchain = renderEngine->createObject<DirectX12Swapchain>();
        if (!swapchain->init(windowID))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize swapchain for window {}"), windowID);
            delete swapchain;
            return false;
        }
        windowData->swapchain = swapchain;
        return true;
    }
    void WindowController_DirectX12::destroyWindowSwapchain(const window_id windowID, WindowData_DirectX12* windowData)
    {
        if (windowData->swapchain != nullptr)
        {
            delete windowData->swapchain;
            windowData->swapchain = nullptr;
        }
    }

    bool WindowController_DirectX12::createWindowSwapchains()
    {
        for (const auto& windowID : getWindowIDs())
        {
            if (!createWindowSwapchain(windowID, getWindowData<WindowData_DirectX12>(windowID)))
            {
                JUTILS_LOG(error, JSTR("Failed to create DirectX12 swapchain"));
                return false;
            }
        }
        return true;
    }
    void WindowController_DirectX12::destroyWindowSwapchains()
    {
        for (const auto& windowID : getWindowIDs())
        {
            destroyWindowSwapchain(windowID, getWindowData<WindowData_DirectX12>(windowID));
        }
    }
}

#endif

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX12)

#include "WindowController_DirectX12.h"

#include "../RenderEngine_DirectX12.h"
#include "../DirectX12Objects/DirectX12Swapchain.h"

namespace JumaRenderEngine
{
    WindowController_DirectX12::~WindowController_DirectX12()
    {
        clearData_DirectX12();
    }

    void WindowController_DirectX12::clearData_DirectX12()
    {
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

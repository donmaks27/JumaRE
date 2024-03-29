﻿// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "JumaRE/window/WindowController.h"

#include <Windows.h>

#include "JumaRE/RenderAPI.h"

struct IDXGISwapChain1;

namespace JumaRenderEngine
{
    class RenderEngine_DirectX11;

    struct WindowData_DirectX11 : WindowData
    {
        HWND windowHandler = nullptr;
        IDXGISwapChain1* swapchain = nullptr;
    };

    class WindowController_DirectX11 : public WindowController
    {
        using Super = WindowController;

        friend RenderEngine_DirectX11;

    public:
        WindowController_DirectX11() = default;
        virtual ~WindowController_DirectX11() override;

        using WindowDataType = WindowData_DirectX11;


        virtual void onFinishWindowRender(window_id windowID) override;

    protected:

        static constexpr RenderAPI API = RenderAPI::DirectX11;


        virtual void clearWindowDataInternal(WindowData* windowData) override;

        bool createWindowSwapchain(window_id windowID, WindowData_DirectX11* windowData);

        virtual void onWindowResized(WindowData* windowData) override;

    private:

        void clearData_DirectX11();

        void destroyWindowSwapchain(WindowData_DirectX11* windowData);
        bool createWindowSwapchains();
        void destroyWindowSwapchains();
    };
}

#endif

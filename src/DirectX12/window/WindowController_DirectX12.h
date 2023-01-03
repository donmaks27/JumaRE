// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "JumaRE/window/WindowController.h"

#include <Windows.h>

#include "JumaRE/RenderAPI.h"

namespace JumaRenderEngine
{
    class DirectX12Swapchain;
    class RenderEngine_DirectX12;

    struct WindowData_DirectX12 : WindowData
    {
        HWND windowHandler = nullptr;

        DirectX12Swapchain* swapchain = nullptr;
    };

    class WindowController_DirectX12 : public WindowController
    {
        using Super = WindowController;

        friend RenderEngine_DirectX12;

    public:
        WindowController_DirectX12() = default;
        virtual ~WindowController_DirectX12() override;

        using WindowDataType = WindowData_DirectX12;
        
    protected:

        static constexpr RenderAPI API = RenderAPI::DirectX12;


        virtual void clearWindowDataInternal(WindowData* windowData) override;

        bool createWindowSwapchain(window_id windowID, WindowData_DirectX12* windowData);

    private:

        void clearData_DirectX12();

        void destroyWindowSwapchain(WindowData_DirectX12* windowData);
        bool createWindowSwapchains();
        void destroyWindowSwapchains();
    };
}

#endif

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "../../../include/JumaRE/window/WindowController.h"

#include <Windows.h>

#include "../../../include/JumaRE/RenderAPI.h"

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
        

        bool isTearingSupported() const { return m_TearingSupported; }

    protected:

        static constexpr RenderAPI API = RenderAPI::DirectX12;


        virtual bool initWindowController() override;

        virtual void destroyWindowInternal(window_id windowID, WindowData* windowData) override;

        bool createWindowSwapchain(window_id windowID, WindowData_DirectX12* windowData);

    private:

        bool m_TearingSupported = false;


        void clearData_DirectX12();

        void destroyWindowSwapchain(window_id windowID, WindowData_DirectX12* windowData);
        bool createWindowSwapchains();
        void destroyWindowSwapchains();
    };
}

#endif

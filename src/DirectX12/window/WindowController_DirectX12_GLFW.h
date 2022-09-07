// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12) && defined(GLFW_ENABLED)

#include "WindowController_DirectX12.h"
#include "../../GLFW/WindowController_GLFW.h"

namespace JumaRenderEngine
{
    using WindowData_DirectX12_GLFW = WindowData_GLFW<WindowData_DirectX12>;

    class WindowController_DirectX12_GLFW final : public WindowController_GLFW<WindowController_DirectX12>
    {
        using Super = WindowController_GLFW<WindowController_DirectX12>;

    public:
        WindowController_DirectX12_GLFW() = default;
        virtual ~WindowController_DirectX12_GLFW() override;

        using WindowDataType = WindowData_DirectX12_GLFW;


        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }

    protected:

        virtual WindowData* createWindowInternal(window_id windowID, const WindowCreateInfo& createInfo) override;
        virtual void destroyWindowInternal(window_id windowID, WindowData* windowData) override;

        virtual WindowData* getWindowData(const window_id windowID) override { return m_Windows.find(windowID); }

    private:

        jmap<window_id, WindowDataType> m_Windows;


        void clearData_DirectX12_GLFW();
    };
}

#endif

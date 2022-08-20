// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11) && defined(JUMARE_ENABLE_LIB_GLFW)

#include "WindowController_DirectX11.h"
#include "../../GLFW/WindowController_GLFW.h"

namespace JumaRenderEngine
{
    using WindowData_DirectX11_GLFW = WindowData_GLFW<WindowData_DirectX11>;

    class WindowController_DirectX11_GLFW final : public WindowController_GLFW<WindowController_DirectX11>
    {
        using Super = WindowController_GLFW<WindowController_DirectX11>;

    public:
        WindowController_DirectX11_GLFW() = default;
        virtual ~WindowController_DirectX11_GLFW() override;

        using WindowDataType = WindowData_DirectX11_GLFW;


        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }

    protected:

        virtual WindowData* createWindowInternal(window_id windowID, const WindowCreateInfo& createInfo) override;
        virtual void destroyWindowInternal(window_id windowID, WindowData* windowData) override;

        virtual WindowData* getWindowData(const window_id windowID) override { return m_Windows.find(windowID); }

    private:

        jmap<window_id, WindowData_DirectX11_GLFW> m_Windows;


        void clearData_DirectX11_GLFW();
    };
}

#endif

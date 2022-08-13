// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11) && defined(JUMARE_ENABLE_LIB_GLFW)

#include "WindowController_DirectX11.h"
#include "../../GLFW/WindowController_GLFW.h"

struct GLFWwindow;

namespace JumaRenderEngine
{
    struct WindowData_DirectX11_GLFW : WindowData_DirectX11, WindowData_GLFW
    {
    };

    class WindowController_DirectX11_GLFW final : public WindowController_DirectX11, protected WindowController_GLFW
    {
        using Super = WindowController_DirectX11;

    public:
        WindowController_DirectX11_GLFW() = default;
        virtual ~WindowController_DirectX11_GLFW() override;

        virtual void destroyWindow(window_id windowID) override;

        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }
        virtual jarray<window_id> getWindowIDs() const override { return m_Windows.getKeys(); }

        virtual bool shouldCloseWindow(window_id windowID) const override;

        virtual void updateWindows() override;

    protected:

        virtual bool initWindowController() override;

        virtual WindowData* createWindowInternal(window_id windowID, const WindowInitProperties& properties) override;

        virtual WindowData* getWindowData(const window_id windowID) override { return m_Windows.find(windowID); }
        
        virtual void GLFW_onWindowResized(WindowData* windowData, const math::uvector2& size) override { updateWindowSize(windowData->windowID, size); }
        virtual void GLFW_onWindowMinimizationChanged(WindowData* windowData, const bool minimized) override { updateWindowMinimization(windowData->windowID, minimized); }

        virtual void setWindowTitleInternal(WindowData* windowData, const jstring& title) override { GLFW_setWindowTitle(reinterpret_cast<const WindowData_DirectX11_GLFW*>(windowData), title); }

    private:

        jmap<window_id, WindowData_DirectX11_GLFW> m_Windows;

        
        void clearGLFW();

        void clearWindowDataGLFW(window_id windowID, WindowData_DirectX11_GLFW& windowData);
    };
}

#endif

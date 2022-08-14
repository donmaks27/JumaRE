// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12) && defined(JUMARE_ENABLE_LIB_GLFW)

#include "WindowController_DirectX12.h"
#include "../../GLFW/WindowController_GLFW.h"

namespace JumaRenderEngine
{
    struct WindowData_DirectX12_GLFW : WindowData_DirectX12, WindowData_GLFW
    {
    };

    class WindowController_DirectX12_GLFW final : public WindowController_DirectX12, protected WindowController_GLFW
    {
        using Super = WindowController_DirectX12;

    public:
        WindowController_DirectX12_GLFW() = default;
        virtual ~WindowController_DirectX12_GLFW() override;
        
        virtual monitor_id getPrimaryMonitorID() const override { return GLFW_getPrimaryMonitorID(); }
        virtual const MonitorData* findMonitorData(const monitor_id monitorID) const override { return GLFW_findMonitorData(monitorID); }
        virtual jarray<window_id> getMonitorIDs() const override { return GLFW_getMonitorIDs(); }

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
        virtual void GLFW_onWindowModeChanged(WindowData* windowData, const WindowMode mode) override { onWindowModeChanged(windowData, mode); }

        virtual void setWindowTitleInternal(WindowData* windowData, const jstring& title) override { GLFW_setWindowTitle(reinterpret_cast<const WindowData_DirectX12_GLFW*>(windowData), title); }
        virtual bool setWindowModeInternal(WindowData* windowData, const WindowMode mode, const monitor_id monitorID) override
        {
            return GLFW_setWindowMode(reinterpret_cast<WindowData_DirectX12_GLFW*>(windowData), mode, monitorID);
        }

    private:

        jmap<window_id, WindowData_DirectX12_GLFW> m_Windows;


        void clearGLFW();

        void clearWindowDataGLFW(window_id windowID, WindowData_DirectX12_GLFW& windowData);
    };
}

#endif

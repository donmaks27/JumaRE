// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"
#include "../RenderEngineContextObject.h"

#include <jutils/jarray.h>
#include <jutils/jdelegate_multicast.h>
#include <jutils/jmap.h>
#include <jutils/math/vector2.h>

#include "window_id.h"
#include "WindowMode.h"
#include "../texture/TextureSamples.h"

namespace JumaRenderEngine
{
    class RenderTarget;
    class WindowController;

    struct WindowInitProperties
    {
        jstring title = JSTR("JumaRE");
        math::uvector2 size = { 0, 0 };
        TextureSamples samples = TextureSamples::X1;
    };

    struct MonitorData
    {
        monitor_id monitorID = monitor_id_INVALID;

        jstring name = "";
        math::uvector2 size = { 0, 0 };
    };

    struct WindowData
    {
        window_id windowID = window_id_INVALID;
        RenderTarget* windowRenderTarget = nullptr;

        math::uvector2 desiredSize = { 0, 0 };
        math::uvector2 actualSize = { 0, 0 };
        TextureSamples samples = TextureSamples::X1;
        jstring title;
        WindowMode mode = WindowMode::Normal;

        bool minimized = false;
    };

    CREATE_JUTILS_MULTICAST_DELEGATE_TwoParams(OnWindowControllerWindowEvent, WindowController*, windowController, const WindowData*, windowData);

    class WindowController : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        WindowController() = default;
        virtual ~WindowController() override = default;

        OnWindowControllerWindowEvent OnWindowPropertiesChanged;


        virtual monitor_id getPrimaryMonitorID() const = 0;
        virtual const MonitorData* findMonitorData(monitor_id monitorID) const = 0;
        virtual jarray<window_id> getMonitorIDs() const = 0;

        bool createWindow(window_id windowID, const WindowInitProperties& properties);
        virtual void destroyWindow(window_id windowID) = 0;

        bool createRenderTargets();
        void destroyRenderTargets();

        virtual const WindowData* findWindowData(window_id windowID) const = 0;
        template<typename T, TEMPLATE_ENABLE(is_base<WindowData, T>)>
        const T* findWindowData(const window_id windowID) const { return reinterpret_cast<const T*>(findWindowData(windowID)); }
        virtual jarray<window_id> getWindowIDs() const = 0;
        
        virtual bool getActualWindowSize(window_id windowID, math::uvector2& outSize) const;

        virtual bool shouldCloseWindow(window_id windowID) const = 0;

        virtual bool onStartRender() { return !isAllWindowsMinimized(); }
        virtual bool onStartWindowRender(const window_id windowID) { return !isWindowMinimized(windowID); }
        virtual void onFinishWindowRender(window_id windowID) {}
        virtual void onFinishRender() {}
        virtual void updateWindows();

        bool isAllWindowsMinimized() const { return m_WindowsCount == m_MinimizedWindowsCount; }
        bool isWindowMinimized(window_id windowID) const;

        bool setWindowTitle(window_id windowID, const jstring& title);
        bool setWindowMode(window_id windowID, WindowMode mode, monitor_id monitorID = monitor_id_INVALID);

    protected:

        virtual bool initWindowController() { return true; }

        virtual WindowData* createWindowInternal(window_id windowID, const WindowInitProperties& properties) = 0;

        void clearWindowData(window_id windowID, WindowData& windowData);

        virtual WindowData* getWindowData(window_id windowID) = 0;
        template<typename T, TEMPLATE_ENABLE(is_base<WindowData, T>)>
        T* getWindowData(const window_id windowID) { return reinterpret_cast<T*>(getWindowData(windowID)); }

        void updateWindowSize(window_id windowID, const math::uvector2& size);
        void updateWindowMinimization(window_id windowID, bool minimized);

        virtual void setWindowTitleInternal(WindowData* windowData, const jstring& title) = 0;
        virtual bool setWindowModeInternal(WindowData* windowData, WindowMode mode, monitor_id monitorID) = 0;

        virtual void onWindowResized(WindowData* windowData) {}
        virtual void onWindowMinimizationChanged(WindowData* windowData);
        virtual void onWindowModeChanged(WindowData* windowData, WindowMode mode);
        
    private:

        jmap<window_id, math::uvector2> m_ChangedWindowSizes;

        uint8 m_WindowsCount = 0;
        uint8 m_MinimizedWindowsCount = 0;


        bool createRenderTarget(window_id windowID, WindowData& windowData);
        void destroyRenderTarget(window_id windowID, WindowData& windowData);
    };
}

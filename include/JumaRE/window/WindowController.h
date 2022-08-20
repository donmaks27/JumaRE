// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"
#include "../RenderEngineContextObject.h"

#include <jutils/jarray.h>
#include <jutils/jdelegate_multicast.h>
#include <jutils/jmap.h>
#include <jutils/juid.h>
#include <jutils/math/vector2.h>

#include "WindowMode.h"
#include "window_id.h"
#include "../texture/TextureSamples.h"

namespace JumaRenderEngine
{
    class RenderTarget;
    class WindowController;

    struct WindowData
    {
        window_id windowID = window_id_INVALID;

        RenderTarget* windowRenderTarget = nullptr;

        math::uvector2 desiredSize = { 0, 0 };
        math::uvector2 size = { 0, 0 };
        TextureSamples samples = TextureSamples::X1;

        bool minimized = false;
    };
    
    struct WindowCreateInfo
    {
        jstring title = JSTR("JumaRE");
        math::uvector2 size = { 0, 0 };
        TextureSamples samples = TextureSamples::X1;
    };

    CREATE_JUTILS_MULTICAST_DELEGATE_TwoParams(OnWindowControllerWindowEvent, WindowController*, windowController, const WindowData*, windowData);

    class WindowController : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        WindowController() = default;
        virtual ~WindowController() override;

        using WindowDataType = WindowData;

        OnWindowControllerWindowEvent OnWindowPropertiesChanged;


        window_id createWindow(const WindowCreateInfo& createInfo);
        void destroyWindow(window_id windowID);

        const jarray<window_id>& getWindowIDs() const { return m_CreatedWindowIDs; }
        window_id getMainWindowID() const { return m_MainWindowID; }
        virtual const WindowData* findWindowData(window_id windowID) const = 0;
        template<typename T, TEMPLATE_ENABLE(is_base<WindowData, T>)>
        const T* findWindowData(const window_id windowID) const { return reinterpret_cast<const T*>(findWindowData(windowID)); }

        virtual bool shouldCloseWindow(window_id windowID) const = 0;

        bool isWindowMinimized(window_id windowID) const;
        bool isAllWindowsMinimized() const { return static_cast<uint8>(m_CreatedWindowIDs.getSize()) == m_MinimizedWindowsCount; }

        bool setMainWindowMode(WindowMode windowMode);
        WindowMode getMainWindowMode() const { return m_MainWindowMode; }

        virtual bool onStartRender() { return !isAllWindowsMinimized(); }
        virtual bool onStartWindowRender(const window_id windowID) { return !isWindowMinimized(windowID); }
        virtual void onFinishWindowRender(window_id windowID) {}
        virtual void onFinishRender() {}
        virtual void updateWindows();

    protected:

        virtual bool initWindowController() { return true; }

        virtual WindowData* createWindowInternal(window_id windowID, const WindowCreateInfo& createInfo) = 0;
        virtual void destroyWindowInternal(window_id windowID, WindowData* windowData);

        virtual WindowData* getWindowData(window_id windowID) = 0;
        template<typename T, TEMPLATE_ENABLE(is_base<WindowData, T>)>
        T* getWindowData(const window_id windowID) { return reinterpret_cast<T*>(getWindowData(windowID)); }

        void updateWindowSize(window_id windowID, const math::uvector2& size);
        virtual void onWindowResized(window_id windowID, WindowData* windowData) {}

        void updateWindowMinimization(window_id windowID, bool minimized);
        virtual void onWindowMinimizationChanged(window_id windowID, WindowData* windowData);

        virtual bool setMainWindowModeInternal(WindowMode windowMode) = 0;
        void updateMainWindowMode(WindowMode windowMode);

    private:

        juid<window_id> m_WindowIDs;
        jarray<window_id> m_CreatedWindowIDs;

        window_id m_MainWindowID = window_id_INVALID;
        WindowMode m_MainWindowMode = WindowMode::Normal;

        jmap<window_id, math::uvector2> m_ChangedWindowSizes;

        uint8 m_MinimizedWindowsCount = 0;


        void clearData();

        bool createMainWindow(const WindowCreateInfo& windowInfo);

        bool createRenderTarget(window_id windowID, WindowData* windowData);
        void destroyRenderTarget(window_id windowID, WindowData* windowData);
        bool createRenderTargets();
        void destroyRenderTargets();
    };
}

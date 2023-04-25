// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"
#include "../RenderEngineContextObject.h"

#include <jutils/jdelegate_multicast.h>
#include <jutils/jset.h>
#include <jutils/jstringID.h>
#include <jutils/math/vector2.h>

#include "window_state_enums.h"
#include "window_id.h"
#include "../render_target_id.h"
#include "../input/InputData.h"
#include "../texture/TextureSamples.h"

namespace JumaRenderEngine
{
    class WindowController;

    struct WindowData
    {
        window_id windowID = window_id_INVALID;

        render_target_id windowRenderTargetID = render_target_id_INVALID;
        TextureSamples samples = TextureSamples::X1;

        math::uvector2 desiredSize = { 0, 0 };
        math::uvector2 size = { 0, 0 };
        bool minimized = false;

        math::uvector2 cursorPosition;
        WindowCursorMode cursorMode = WindowCursorMode::Normal;

        InputData inputData;
    };
    
    struct WindowCreateInfo
    {
        jstring title = JSTR("JumaRE");
        math::uvector2 size = { 0, 0 };
        TextureSamples samples = TextureSamples::X1;
    };

    JUTILS_CREATE_MULTICAST_DELEGATE1(OnWindowControllerEvent, WindowController*, windowController);
    JUTILS_CREATE_MULTICAST_DELEGATE2(OnWindowControllerWindowEvent, WindowController*, windowController, const WindowData*, windowData);
    JUTILS_CREATE_MULTICAST_DELEGATE3(OnWindowControllerInput, WindowController*, windowController, const WindowData*, windowData, const InputActionData&, input);
    
    class WindowController : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        WindowController() = default;
        virtual ~WindowController() override;

        using WindowDataType = WindowData;

        OnWindowControllerWindowEvent onWindowCreated;
        OnWindowControllerWindowEvent onWindowDestroying;

        OnWindowControllerWindowEvent onWindowPropertiesChanged;
        OnWindowControllerInput onWindowInput;


        window_id createWindow(const WindowCreateInfo& createInfo) { return createWindow(createInfo, true); }
        void destroyWindow(window_id windowID);

        const jarray<window_id>& getWindowIDs() const { return m_CreatedWindowIDs; }
        window_id getMainWindowID() const { return m_MainWindowID; }
        virtual const WindowData* findWindowData(window_id windowID) const = 0;
        template<typename T> requires is_base_class<WindowData, T>
        const T* findWindowData(const window_id windowID) const { return reinterpret_cast<const T*>(findWindowData(windowID)); }

        virtual bool shouldCloseWindow(window_id windowID) const = 0;
        bool isMainWindowClosed() const { return shouldCloseWindow(getMainWindowID()); }

        bool isWindowMinimized(window_id windowID) const;
        bool isAllWindowsMinimized() const { return static_cast<uint8>(m_CreatedWindowIDs.getSize()) == m_MinimizedWindowsCount; }

        bool setMainWindowMode(WindowMode windowMode);
        WindowMode getMainWindowMode() const { return m_MainWindowMode; }

        window_id getFocusedWindowID() const { return m_FocusedWindowID; }

        void setCursorMode(window_id windowID, WindowCursorMode mode);
        WindowCursorMode getCursorMode(window_id windowID) const;

        const jset<gamepad_index_type>& getConnectedGamepads() const { return m_ConnectedGamepads; }

        virtual bool onStartRender() { return !isAllWindowsMinimized(); }
        virtual bool onStartWindowRender(const window_id windowID) { return !isWindowMinimized(windowID); }
        virtual void onFinishWindowRender(window_id windowID) {}
        virtual void onFinishRender() {}
        virtual void updateWindows();

    protected:

        virtual bool initWindowController() { return true; }

        virtual WindowData* createWindowInternal(window_id windowID, const WindowCreateInfo& createInfo) = 0;
        virtual void markWindowShouldClose(WindowData* windowData) = 0;
        virtual void destroyWindowInternal(window_id windowID, WindowData* windowData) = 0;
        virtual void clearWindowDataInternal(WindowData* windowData);

        virtual WindowData* getWindowData(window_id windowID) = 0;
        template<typename T> requires is_base_class<WindowData, T>
        T* getWindowData(const window_id windowID) { return reinterpret_cast<T*>(getWindowData(windowID)); }

        void updateWindowSize(window_id windowID, const math::uvector2& size);
        virtual void onWindowResized(WindowData* windowData) {}

        void updateWindowMinimization(window_id windowID, bool minimized);
        virtual void onWindowMinimizationChanged(WindowData* windowData);

        virtual bool setMainWindowModeInternal(WindowMode windowMode) = 0;
        void updateMainWindowMode(WindowMode windowMode);

        void updateWindowFocused(window_id focusedWindowID, bool focused);

        virtual void onWindowCursorModeChanged(WindowData* windowData) {}

        void updateGamepadConnected(gamepad_index_type gamepadIndex, bool connected);

        void updateWindowInputButtonState(window_id windowID, InputDevice device, InputButton button, InputButtonAction action, 
            input_mods_type mods = 0);
        void updateWindowInputAxisState(window_id windowID, InputDevice device, InputAxis axis, const math::vector2& value, 
            input_mods_type mods = 0);

    private:

        struct ReceivedWindowButtonInput
        {
            window_id windowID = window_id_INVALID;
            InputDevice device = InputDevice::NONE;
            InputButton button = InputButton::NONE;
            constexpr bool operator<(const ReceivedWindowButtonInput& input) const
            {
                if (windowID != input.windowID)
                {
                    return windowID < input.windowID;
                }
                return device != input.device ? device < input.device : button < input.button;
            }
        };
        struct ReceivedWindowAxisInput
        {
            window_id windowID = window_id_INVALID;
            InputDevice device = InputDevice::NONE;
            InputAxis axis = InputAxis::NONE;
            constexpr bool operator<(const ReceivedWindowAxisInput& input) const
            {
                if (windowID != input.windowID)
                {
                    return windowID < input.windowID;
                }
                return device != input.device ? device < input.device : axis < input.axis;
            }
        };

        juid<window_id> m_WindowIDs;
        jarray<window_id> m_CreatedWindowIDs;
        jset<gamepad_index_type> m_ConnectedGamepads;

        jmap<window_id, math::uvector2> m_ChangedWindowSizes;
        jmap<ReceivedWindowButtonInput, InputButtonAction> m_ReceivedButtonInput;
        jmap<ReceivedWindowAxisInput, math::vector2> m_ReceivedAxisInput;

        window_id m_MainWindowID = window_id_INVALID;
        uint8 m_MinimizedWindowsCount = 0;
        WindowMode m_MainWindowMode = WindowMode::Normal;
        window_id m_FocusedWindowID = window_id_INVALID;


        void clearData();

        bool createMainWindow(const WindowCreateInfo& windowInfo);
        window_id createWindow(const WindowCreateInfo& createInfo, bool callEvent);

        bool createRenderTarget(WindowData* windowData);
        void destroyRenderTarget(WindowData* windowData);
        bool createRenderTargets();
        void destroyRenderTargets();
    };
}

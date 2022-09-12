// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "../core.h"
#include "../RenderEngineContextObject.h"

#include <jutils/jarray.h>
#include <jutils/jdelegate_multicast.h>
#include <jutils/jmap.h>
#include <jutils/jset.h>
#include <jutils/jstringID.h>
#include <jutils/juid.h>
#include <jutils/math/vector2.h>

#include "WindowMode.h"
#include "window_id.h"
#include "../input/InputData.h"
#include "../texture/TextureSamples.h"

namespace JumaRenderEngine
{
    class RenderTarget;
    class WindowController;

    struct WindowData
    {
        window_id windowID = window_id_INVALID;

        RenderTarget* windowRenderTarget = nullptr;
        TextureSamples samples = TextureSamples::X1;
        jstringID pipelineStageName = jstringID_NONE;

        math::uvector2 desiredSize = { 0, 0 };
        math::uvector2 size = { 0, 0 };
        bool minimized = false;

        math::uvector2 cursorPosition;
        InputData inputData;
    };
    
    struct WindowCreateInfo
    {
        jstring title = JSTR("JumaRE");
        math::uvector2 size = { 0, 0 };
        TextureSamples samples = TextureSamples::X1;
    };

    CREATE_JUTILS_MULTICAST_DELEGATE_TwoParams(OnWindowControllerWindowEvent, WindowController*, windowController, const WindowData*, windowData);
    CREATE_JUTILS_MULTICAST_DELEGATE_FiveParams(OnWindowControllerInputButtonEvent, WindowController*, windowController, const WindowData*, windowData, InputDeviceType, device, InputButton, button, InputButtonAction, action);
    CREATE_JUTILS_MULTICAST_DELEGATE_FiveParams(OnWindowControllerInputAxisEvent, WindowController*, windowController, const WindowData*, windowData, InputDeviceType, device, InputAxis, axis, float, value);
    CREATE_JUTILS_MULTICAST_DELEGATE_FiveParams(OnWindowControllerInputAxis2DEvent, WindowController*, windowController, const WindowData*, windowData, InputDeviceType, device, InputAxis, axis, const math::vector2&, value);

    class WindowController : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        WindowController() = default;
        virtual ~WindowController() override;

        using WindowDataType = WindowData;

        OnWindowControllerWindowEvent OnWindowPropertiesChanged;
        OnWindowControllerInputButtonEvent OnInputButton;
        OnWindowControllerInputAxisEvent OnInputAxis;
        OnWindowControllerInputAxis2DEvent OnInputAxis2D;


        window_id createWindow(const WindowCreateInfo& createInfo);
        void destroyWindow(window_id windowID);

        const jarray<window_id>& getWindowIDs() const { return m_CreatedWindowIDs; }
        window_id getMainWindowID() const { return m_MainWindowID; }
        virtual const WindowData* findWindowData(window_id windowID) const = 0;
        template<typename T, TEMPLATE_ENABLE(is_base<WindowData, T>)>
        const T* findWindowData(const window_id windowID) const { return reinterpret_cast<const T*>(findWindowData(windowID)); }

        virtual bool shouldCloseWindow(window_id windowID) const = 0;
        bool isMainWindowClosed() const { return shouldCloseWindow(getMainWindowID()); }

        bool isWindowMinimized(window_id windowID) const;
        bool isAllWindowsMinimized() const { return static_cast<uint8>(m_CreatedWindowIDs.getSize()) == m_MinimizedWindowsCount; }

        bool setMainWindowMode(WindowMode windowMode);
        WindowMode getMainWindowMode() const { return m_MainWindowMode; }

        window_id getFocusedWindowID() const { return m_FocusedWindowID; }

        void setCursorLockedToMainWindow(bool locked);
        bool isCursorLockedToMainWindow() const { return m_CursorLockedToMainWindow; }
        void resetLockedCursorPosition();

        const jset<gamepad_index_type>& getConnectedGamepads() const { return m_ConnectedGamepads; }

        virtual bool onStartRender() { return !isAllWindowsMinimized(); }
        virtual bool onStartWindowRender(const window_id windowID) { return !isWindowMinimized(windowID); }
        virtual void onFinishWindowRender(window_id windowID) {}
        virtual void onFinishRender() {}
        virtual void updateWindows();

    protected:

        virtual bool initWindowController() { return true; }

        virtual WindowData* createWindowInternal(window_id windowID, const WindowCreateInfo& createInfo) = 0;
        virtual void destroyWindowInternal(window_id windowID, WindowData* windowData) = 0;
        virtual void clearWindowDataInternal(window_id windowID, WindowData* windowData);

        virtual WindowData* getWindowData(window_id windowID) = 0;
        template<typename T, TEMPLATE_ENABLE(is_base<WindowData, T>)>
        T* getWindowData(const window_id windowID) { return reinterpret_cast<T*>(getWindowData(windowID)); }

        void updateWindowSize(window_id windowID, const math::uvector2& size);
        virtual void onWindowResized(window_id windowID, WindowData* windowData) {}

        void updateWindowMinimization(window_id windowID, bool minimized);
        virtual void onWindowMinimizationChanged(window_id windowID, WindowData* windowData);

        virtual bool setMainWindowModeInternal(WindowMode windowMode) = 0;
        void updateMainWindowMode(WindowMode windowMode);

        void updateWindowFocused(window_id focusedWindowID, bool focused);
        void updateWindowCursorPosition(window_id windowID, const math::ivector2& position, const math::ivector2& offset);
        virtual bool setCursorLockedToMainWindowInternal(bool locked) = 0;

        void updateGamepadConnected(gamepad_index_type gamepadIndex, bool connected);

        void updateWindowInputButtonState(window_id windowID, InputDeviceType device, InputButton button, InputButtonAction action, 
            input_mods_type mods = 0);
        void updateWindowInputAxisState(window_id windowID, InputDeviceType device, InputAxis axis, const math::vector2& value, 
            input_mods_type mods = 0);

    private:

        struct ReceivedWindowButtonInput
        {
            window_id windowID = window_id_INVALID;
            InputDeviceType device = InputDeviceType::NONE;
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
            InputDeviceType device = InputDeviceType::NONE;
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
        bool m_CursorLockedToMainWindow = false;


        void clearData();

        bool createMainWindow(const WindowCreateInfo& windowInfo);

        bool createRenderTarget(window_id windowID, WindowData* windowData);
        void destroyRenderTarget(window_id windowID, WindowData* windowData);
        bool createRenderTargets();
        void destroyRenderTargets();

        bool addPipelineStage(window_id windowID, const WindowData* windowData, bool rebuildPipeline = false);
        bool removePipelineStage(window_id windowID, const WindowData* windowData, bool rebuildPipeline = false);
        void addPipelineStages();
    };
}

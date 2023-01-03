// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "JumaRE/window/WindowController.h"

#include "JumaRE/RenderEngine.h"
#include "JumaRE/RenderPipeline.h"
#include "JumaRE/RenderTarget.h"

namespace JumaRenderEngine
{
    WindowController::~WindowController()
    {
        clearData();
    }

    void WindowController::clearData()
    {
        m_WindowIDs.reset();
        m_CreatedWindowIDs.clear();
        m_MainWindowID = window_id_INVALID;
    }

    bool WindowController::createMainWindow(const WindowCreateInfo& windowInfo)
    {
        const window_id windowID = createWindow(windowInfo);
        if (windowID == window_id_INVALID)
        {
            return false;
        }
        m_MainWindowID = windowID;
        m_FocusedWindowID = m_MainWindowID;
        return true;
    }
    window_id WindowController::createWindow(const WindowCreateInfo& createInfo)
    {
        if ((createInfo.size.x == 0) || (createInfo.size.y == 0))
        {
            return window_id_INVALID;
        }

        const window_id windowID = m_WindowIDs.getUID();
        WindowData* windowData = createWindowInternal(windowID, createInfo);
        if (windowData == nullptr)
        {
            return window_id_INVALID;
        }
        m_CreatedWindowIDs.add(windowID);
        windowData->windowID = windowID;
        windowData->desiredSize = createInfo.size;
        windowData->size = createInfo.size;
        windowData->samples = createInfo.samples;
        windowData->minimized = false;
        windowData->pipelineStageName = JSTR("window") + TO_JSTR(windowID);

        updateWindowFocused(windowID, true);

        if (!createRenderTarget(windowData))
        {
            destroyWindow(windowID);
            return window_id_INVALID;
        }
        return windowID;
    }
    void WindowController::destroyWindow(const window_id windowID)
    {
        if (windowID == window_id_INVALID)
        {
            return;
        }
        WindowData* windowData = getWindowData(windowID);
        if (windowData == nullptr)
        {
            return;
        }
        if (windowID == getMainWindowID())
        {
            markWindowShouldClose(windowData);
            return;
        }

        if (windowData->minimized)
        {
            m_MinimizedWindowsCount--;
        }
        destroyWindowInternal(windowID, windowData);
        m_CreatedWindowIDs.remove(windowID);
    }
    void WindowController::clearWindowDataInternal(WindowData* windowData)
    {
        destroyRenderTarget(windowData);

        windowData->windowID = window_id_INVALID;
        windowData->desiredSize = { 0, 0 };
        windowData->size = windowData->desiredSize;
        windowData->samples = TextureSamples::X1;
        windowData->minimized = false;
    }

    bool WindowController::createRenderTarget(WindowData* windowData)
    {
        RenderEngine* renderEngine = getRenderEngine();
        if (!renderEngine->isValid())
        {
            return true;
        }

        if (windowData->windowRenderTargetID != render_target_id_INVALID)
        {
            return true;
        }
        RenderTarget* renderTarget = renderEngine->createWindowRenderTarget(windowData->windowID, windowData->samples);
        if (renderTarget == nullptr)
        {
            return false;
        }
        windowData->windowRenderTargetID = renderTarget->getID();
        return true;
    }
    void WindowController::destroyRenderTarget(WindowData* windowData)
    {
        if (windowData->windowRenderTargetID != render_target_id_INVALID)
        {
            RenderEngine* renderEngine = getRenderEngine();
            RenderTarget* renderTarget = renderEngine->getRenderTarget(windowData->windowRenderTargetID);
            renderEngine->destroyRenderTarget(renderTarget);
            windowData->windowRenderTargetID = render_target_id_INVALID;
        }
    }
    bool WindowController::createRenderTargets()
    {
        for (const auto& windowID : getWindowIDs())
        {
            if (!createRenderTarget(getWindowData(windowID)))
            {
                JUTILS_LOG(error, JSTR("Failed to create DirectX11 render target for window {}"), windowID);
                return false;
            }
        }
        return true;
    }
    void WindowController::destroyRenderTargets()
    {
        for (const auto& windowID : getWindowIDs())
        {
            WindowData* windowData = getWindowData(windowID);
            if (windowData != nullptr)
            {
                windowData->windowRenderTargetID = render_target_id_INVALID;
            }
        }
    }

    void WindowController::updateWindowSize(const window_id windowID, const math::uvector2& size)
    {
        m_ChangedWindowSizes.add(windowID, size);
    }

    void WindowController::updateWindowMinimization(const window_id windowID, const bool minimized)
    {
        WindowData* windowData = getWindowData(windowID);
        if (windowData->minimized != minimized)
        {
            windowData->minimized = minimized;
            onWindowMinimizationChanged(windowData);
        }
    }
    void WindowController::onWindowMinimizationChanged(WindowData* windowData)
    {
        if (windowData->minimized)
        {
            m_MinimizedWindowsCount++;
        }
        else
        {
            m_MinimizedWindowsCount--;
        }
    }
    bool WindowController::isWindowMinimized(const window_id windowID) const
    {
        const WindowData* windowData = findWindowData(windowID);
        return (windowData != nullptr) && windowData->minimized;
    }

    bool WindowController::setMainWindowMode(const WindowMode windowMode)
    {
        if (m_MainWindowID == window_id_INVALID)
        {
            JUTILS_LOG(error, JSTR("Main window wasn't created"));
            return false;
        }

        if (m_MainWindowMode != windowMode)
        {
            if (!setMainWindowModeInternal(windowMode))
            {
                return false;
            }
            updateMainWindowMode(windowMode);
        }
        return true;
    }
    void WindowController::updateMainWindowMode(const WindowMode windowMode)
    {
        m_MainWindowMode = windowMode;

        const render_target_id renderTargetID = findWindowData(getMainWindowID())->windowRenderTargetID;
        RenderTarget* renderTarget = getRenderEngine()->getRenderTarget(renderTargetID);
        if (renderTarget != nullptr)
        {
            renderTarget->invalidate();
        }
    }
    
    void WindowController::updateWindowFocused(const window_id focusedWindowID, const bool focused)
    {
        if (focused)
        {
            if (m_FocusedWindowID != focusedWindowID)
            {
                m_FocusedWindowID = focusedWindowID;
            }
        }
        else if (m_FocusedWindowID == focusedWindowID)
        {
            m_FocusedWindowID = window_id_INVALID;
        }
    }

    void WindowController::setCursorMode(const window_id windowID, const WindowCursorMode mode)
    {
        WindowData* windowData = getWindowData(windowID);
        if ((windowData != nullptr) && (windowData->cursorMode != mode))
        {
            windowData->cursorMode = mode;
            onWindowCursorModeChanged(windowData);
        }
    }
    WindowCursorMode WindowController::getCursorMode(window_id windowID) const
    {
        const WindowData* windowData = findWindowData(windowID);
        return windowData != nullptr ? windowData->cursorMode : WindowCursorMode::None;
    }

    void WindowController::updateGamepadConnected(const gamepad_index_type gamepadIndex, const bool connected)
    {
        if (IsGamepadIndexValid(gamepadIndex))
        {
            if (connected)
            {
                m_ConnectedGamepads.add(gamepadIndex);
                JUTILS_LOG(info, JSTR("Gamepad {} connected"), gamepadIndex + 1);
            }
            else
            {
                m_ConnectedGamepads.remove(gamepadIndex);
                JUTILS_LOG(info, JSTR("Gamepad {} disconnected"), gamepadIndex + 1);
            }
        }
    }
    
    void WindowController::updateWindowInputButtonState(const window_id windowID, const InputDevice device, const InputButton button, 
        const InputButtonAction action, const input_mods_type mods)
    {
        WindowData* windowData = getWindowData(windowID);
        if ((windowData != nullptr) && windowData->inputData.setButtonState(device, button, action, mods))
        {
            m_ReceivedButtonInput.add({ windowID, device, button }, action);
        }
    }
    void WindowController::updateWindowInputAxisState(const window_id windowID, const InputDevice device, const InputAxis axis, 
        const math::vector2& value, const input_mods_type mods)
    {
        WindowData* windowData = getWindowData(windowID);
        if ((windowData != nullptr) && windowData->inputData.setAxisState(device, axis, value, mods))
        {
            m_ReceivedAxisInput.add({ windowID, device, axis }, value);
        }
    }

    void WindowController::updateWindows()
    {
        if (!m_ChangedWindowSizes.isEmpty())
        {
            for (const auto& changedWindowSize : m_ChangedWindowSizes)
            {
                const window_id windowID = changedWindowSize.key;
                WindowData* windowData = getWindowData(windowID);
                if (windowData != nullptr)
                {
                    JUTILS_LOG(info, JSTR("Window {} size changed - {}"), windowID, changedWindowSize.value);
                    windowData->size = changedWindowSize.value;
                    if ((windowID == getMainWindowID()) && (getMainWindowMode() != WindowMode::WindowedFullscreen))
                    {
                        windowData->desiredSize = windowData->size;
                    }
                    onWindowResized(windowData);
                    OnWindowPropertiesChanged.call(this, windowData);
                }
            }
            m_ChangedWindowSizes.clear();
        }

        if (!m_ReceivedButtonInput.isEmpty())
        {
            for (const auto& input : m_ReceivedButtonInput)
            {
                const WindowData* windowData = findWindowData(input.key.windowID);
                if (windowData != nullptr)
                {
                    OnInputButton.call(this, windowData, input.key.device, input.key.button, input.value);
                }
            }
            m_ReceivedButtonInput.clear();
        }
        if (!m_ReceivedAxisInput.isEmpty())
        {
            for (const auto& input : m_ReceivedAxisInput)
            {
                const WindowData* windowData = findWindowData(input.key.windowID);
                if (windowData != nullptr)
                {
                    if (IsInputAxis2D(input.key.axis))
                    {
                        OnInputAxis2D.call(this, windowData, input.key.device, input.key.axis, input.value);
                    }
                    else
                    {
                        OnInputAxis.call(this, windowData, input.key.device, input.key.axis, input.value.x);
                    }
                }
            }
            m_ReceivedAxisInput.clear();
        }
    }
}

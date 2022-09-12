﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/window/WindowController.h"

#include "../../include/JumaRE/RenderEngine.h"
#include "../../include/JumaRE/RenderPipeline.h"
#include "../../include/JumaRE/RenderTarget.h"

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

        if (!createRenderTarget(windowID, windowData))
        {
            destroyWindow(windowID);
            return window_id_INVALID;
        }

        addPipelineStage(windowID, windowData, true);
        return windowID;
    }
    void WindowController::destroyWindow(const window_id windowID)
    {
        if ((windowID == window_id_INVALID) || (windowID == getMainWindowID()))
        {
            return;
        }
        WindowData* windowData = getWindowData(windowID);
        if (windowData == nullptr)
        {
            return;
        }

        if (windowData->minimized)
        {
            m_MinimizedWindowsCount--;
        }
        removePipelineStage(windowID, windowData, true);
        destroyWindowInternal(windowID, windowData);
        m_CreatedWindowIDs.remove(windowID);
    }
    void WindowController::clearWindowDataInternal(window_id windowID, WindowData* windowData)
    {
        destroyRenderTarget(windowID, windowData);

        windowData->windowID = window_id_INVALID;
        windowData->desiredSize = { 0, 0 };
        windowData->size = windowData->desiredSize;
        windowData->samples = TextureSamples::X1;
        windowData->minimized = false;
    }

    bool WindowController::createRenderTarget(const window_id windowID, WindowData* windowData)
    {
        RenderEngine* renderEngine = getRenderEngine();
        if (!renderEngine->isValid())
        {
            return true;
        }

        if (windowData->windowRenderTarget != nullptr)
        {
            return true;
        }
        RenderTarget* renderTarget = renderEngine->createWindowRenderTarget(windowID, windowData->samples);
        if (renderTarget == nullptr)
        {
            return false;
        }
        windowData->windowRenderTarget = renderTarget;
        return true;
    }
    void WindowController::destroyRenderTarget(const window_id windowID, WindowData* windowData)
    {
        if (windowData->windowRenderTarget != nullptr)
        {
            getRenderEngine()->destroyRenderTarget(windowData->windowRenderTarget);
            windowData->windowRenderTarget = nullptr;
        }
    }
    bool WindowController::createRenderTargets()
    {
        for (const auto& windowID : getWindowIDs())
        {
            if (!createRenderTarget(windowID, getWindowData(windowID)))
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
                windowData->windowRenderTarget = nullptr;
            }
        }
    }

    bool WindowController::addPipelineStage(const window_id windowID, const WindowData* windowData, 
        const bool rebuildPipeline)
    {
        if (windowData->windowRenderTarget == nullptr)
        {
            return false;
        }
        RenderPipeline* pipeline = getRenderEngine()->getRenderPipeline();
        if (pipeline == nullptr)
        {
            return false;
        }
        if (!pipeline->addPipelineStage(windowData->pipelineStageName, windowData->windowRenderTarget))
        {
            return false;
        }
        if (rebuildPipeline)
        {
            pipeline->buildPipelineQueue();
        }
        return true;
    }
    bool WindowController::removePipelineStage(const window_id windowID, const WindowData* windowData, 
        const bool rebuildPipeline)
    {
        RenderPipeline* pipeline = getRenderEngine()->getRenderPipeline();
        if ((pipeline == nullptr) || !pipeline->removePipelineStage(windowData->pipelineStageName))
        {
            return false;
        }
        if (rebuildPipeline)
        {
            pipeline->buildPipelineQueue();
        }
        return true;
    }
    void WindowController::addPipelineStages()
    {
        for (int32 index = 0; index < m_CreatedWindowIDs.getSize(); index++)
        {
            const window_id windowID = m_CreatedWindowIDs[index];
            addPipelineStage(windowID, findWindowData(windowID), index == (m_CreatedWindowIDs.getSize() - 1));
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
            onWindowMinimizationChanged(windowID, windowData);
        }
    }
    void WindowController::onWindowMinimizationChanged(const window_id windowID, WindowData* windowData)
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

        RenderTarget* renderTarget = findWindowData(getMainWindowID())->windowRenderTarget;
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
    void WindowController::updateWindowCursorPosition(const window_id windowID, const math::ivector2& position, const math::ivector2& offset)
    {
        if (m_CursorLockedToMainWindow && ((windowID != m_MainWindowID) || (m_FocusedWindowID != m_MainWindowID)))
        {
            return;
        }

        WindowData* windowData = getWindowData(windowID);
        if (!m_CursorLockedToMainWindow)
        {
            windowData->cursorPosition = position;
        }
        else
        {
            const math::ivector2 newPosition = offset + windowData->cursorPosition;
            windowData->cursorPosition.x = static_cast<uint32>(math::clamp(newPosition.x, 0, windowData->size.x));
            windowData->cursorPosition.y = static_cast<uint32>(math::clamp(newPosition.y, 0, windowData->size.y));
            updateWindowInputAxisState(windowID, InputDeviceType::Mouse, InputAxis::Mouse2D, offset, 0);
        }
    }
    void WindowController::setCursorLockedToMainWindow(const bool locked)
    {
        if ((m_CursorLockedToMainWindow != locked) && setCursorLockedToMainWindowInternal(locked))
        {
            m_CursorLockedToMainWindow = locked;
            resetLockedCursorPosition();
        }
    }
    void WindowController::resetLockedCursorPosition()
    {
        if (m_CursorLockedToMainWindow)
        {
            WindowData* windowData = getWindowData(getMainWindowID());
            windowData->cursorPosition = windowData->size / 2;
        }
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
    
    void WindowController::updateWindowInputButtonState(const window_id windowID, const InputDeviceType device, const InputButton button, 
        const InputButtonAction action, const input_mods_type mods)
    {
        WindowData* windowData = getWindowData(windowID);
        if ((windowData != nullptr) && windowData->inputData.setButtonState(device, button, action, mods))
        {
            m_ReceivedButtonInput.add({ windowID, device, button }, action);
        }
    }
    void WindowController::updateWindowInputAxisState(const window_id windowID, const InputDeviceType device, const InputAxis axis, 
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
                    JUTILS_LOG(info, JSTR("Window {} size changed - {}"), windowID, changedWindowSize.value.toString());
                    windowData->size = changedWindowSize.value;
                    if ((windowID == getMainWindowID()) && (getMainWindowMode() != WindowMode::WindowedFullscreen))
                    {
                        windowData->desiredSize = windowData->size;
                    }
                    onWindowResized(windowID, windowData);
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

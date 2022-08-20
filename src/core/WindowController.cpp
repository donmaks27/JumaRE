// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/window/WindowController.h"

#include "../../include/JumaRE/RenderEngine.h"
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

        if (!createRenderTarget(windowID, windowData))
        {
            destroyWindow(windowID);
            return window_id_INVALID;
        }
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
        destroyWindowInternal(windowID, windowData);
        m_CreatedWindowIDs.remove(windowID);
    }
    void WindowController::destroyWindowInternal(const window_id windowID, WindowData* windowData)
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
            delete windowData->windowRenderTarget;
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
            destroyRenderTarget(windowID, getWindowData(windowID));
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
                    JUTILS_LOG(info, JSTR("Window {} size changed - {{ {}; {} }}"), windowID, changedWindowSize.value.x, changedWindowSize.value.y);
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
    }
}

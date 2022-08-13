// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/window/WindowController.h"

#include "../../include/JumaRE/RenderEngine.h"
#include "../../include/JumaRE/RenderTarget.h"

namespace JumaRenderEngine
{
    bool WindowController::createWindow(const window_id windowID, const WindowProperties& properties)
    {
        WindowData* windowData = createWindowInternal(windowID, properties);
        if (windowData == nullptr)
        {
            return false;
        }

        windowData->windowID = windowID;
        windowData->title = properties.title;
        windowData->actualSize = properties.size;
        windowData->samples = properties.samples;
        if (!createRenderTarget(windowID, *windowData))
        {
            destroyWindow(windowID);
            return false;
        }

        m_WindowsCount++;
        return true;
    }

    void WindowController::clearWindowData(const window_id windowID, WindowData& windowData)
    {
        m_WindowsCount--;
        if (windowData.minimized)
        {
            m_MinimizedWindowsCount--;
        }

        destroyRenderTarget(windowID, windowData);
    }

    bool WindowController::createRenderTarget(const window_id windowID, WindowData& windowData)
    {
        RenderEngine* renderEngine = getRenderEngine();
        if (!renderEngine->isValid())
        {
            return true;
        }

        if (windowData.windowRenderTarget != nullptr)
        {
            return true;
        }
        RenderTarget* renderTarget = renderEngine->createWindowRenderTarget(windowID, windowData.samples);
        if (renderTarget == nullptr)
        {
            return false;
        }
        windowData.windowRenderTarget = renderTarget;
        return true;
    }
    void WindowController::destroyRenderTarget(const window_id windowID, WindowData& windowData)
    {
        if (windowData.windowRenderTarget != nullptr)
        {
            delete windowData.windowRenderTarget;
            windowData.windowRenderTarget = nullptr;
        }
    }

    bool WindowController::createRenderTargets()
    {
        for (const auto& windowID : getWindowIDs())
        {
            if (!createRenderTarget(windowID, *getWindowData(windowID)))
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
            destroyRenderTarget(windowID, *getWindowData(windowID));
        }
    }

    bool WindowController::getActualWindowSize(const window_id windowID, math::uvector2& outSize) const
    {
        const WindowData* windowData = findWindowData(windowID);
        if (windowData == nullptr)
        {
            return false;
        }
        outSize = windowData->actualSize;
        return true;
    }

    void WindowController::updateWindowSize(const window_id windowID, const math::uvector2& size)
    {
        m_ChangedWindowSizes.add(windowID, size);
    }
    void WindowController::updateWindows()
    {
        if (!m_ChangedWindowSizes.isEmpty())
        {
            for (const auto& changedWindowSize : m_ChangedWindowSizes)
            {
                WindowData* windowData = getWindowData(changedWindowSize.key);
                if (windowData != nullptr)
                {
                    JUTILS_LOG(info, JSTR("Window {} size changed - {{ {}; {} }}"), windowData->windowID, changedWindowSize.value.x, changedWindowSize.value.y);
                    windowData->actualSize = changedWindowSize.value;
                    onWindowResized(windowData);
                    OnWindowPropertiesChanged.call(this, windowData);
                }
            }
            m_ChangedWindowSizes.clear();
        }
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

    bool WindowController::setWindowTitle(const window_id windowID, const jstring& title)
    {
        WindowData* windowData = getWindowData(windowID);
        if (windowData == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Can't find window {}"), windowID);
            return false;
        }

        windowData->title = title;
        setWindowTitleInternal(windowData, title);
        return true;
    }
}

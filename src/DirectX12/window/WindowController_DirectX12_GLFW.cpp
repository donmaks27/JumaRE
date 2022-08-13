// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX12) && defined(JUMARE_ENABLE_LIB_GLFW)

#include "WindowController_DirectX12_GLFW.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace JumaRenderEngine
{
    WindowController_DirectX12_GLFW::~WindowController_DirectX12_GLFW()
    {
        clearGLFW();
    }

    bool WindowController_DirectX12_GLFW::initWindowController()
    {
        if (!Super::initWindowController())
        {
            return false;
        }
        if (!GLFW_init(RenderAPI::DirectX12))
        {
            JUTILS_LOG(error, JSTR("Failed to initialize GLFW"));
            return false;
        }
        return true;
    }

    void WindowController_DirectX12_GLFW::clearGLFW()
    {
        if (!m_Windows.isEmpty())
        {
            for (auto& window : m_Windows)
            {
                clearWindowDataGLFW(window.key, window.value);
            }
            m_Windows.clear();
        }
        GLFW_terminate();
    }

    WindowData* WindowController_DirectX12_GLFW::createWindowInternal(const window_id windowID, const WindowInitProperties& properties)
    {
        if (windowID == window_id_INVALID)
        {
            JUTILS_LOG(error, JSTR("Invalid window ID"));
            return nullptr;
        }
        if (m_Windows.contains(windowID))
        {
            JUTILS_LOG(error, JSTR("Window {} already created"), windowID);
            return nullptr;
        }

        WindowData_DirectX12_GLFW* windowData = &m_Windows.add(windowID);
        if (!GLFW_createWindow(windowData, properties.size, properties.title))
        {
            JUTILS_LOG(error, JSTR("Failed to create window {}"), windowID);
            m_Windows.remove(windowID);
            return nullptr;
        }
        windowData->windowHandler = glfwGetWin32Window(windowData->windowGLFW);

        if (!createWindowSwapchain(windowID, windowData))
        {
            clearWindowDataGLFW(windowID, *windowData);
            m_Windows.remove(windowID);
            return nullptr;
        }
        return windowData;
    }

    void WindowController_DirectX12_GLFW::destroyWindow(const window_id windowID)
    {
        WindowData_DirectX12_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Can't find window {}"), windowID);
            return;
        }

        clearWindowDataGLFW(windowID, *windowData);
        m_Windows.remove(windowID);
    }
    void WindowController_DirectX12_GLFW::clearWindowDataGLFW(const window_id windowID, WindowData_DirectX12_GLFW& windowData)
    {
        clearWindowDataDirectX(windowID, windowData);
        GLFW_destroyWindow(&windowData);
    }

    bool WindowController_DirectX12_GLFW::shouldCloseWindow(const window_id windowID) const
    {
        const WindowData_DirectX12_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Can't find window {}"), windowID);
            return false;
        }
        return GLFW_shouldCloseWindow(windowData);
    }

    void WindowController_DirectX12_GLFW::updateWindows()
    {
        GLFW_pushWindowEvents();
        Super::updateWindows();
    }
}

#endif

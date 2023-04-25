// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_DX12) && defined(GLFW_ENABLED)

#include "WindowController_DirectX12_GLFW.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace JumaRenderEngine
{
    WindowController_DirectX12_GLFW::~WindowController_DirectX12_GLFW()
    {
        clearData_DirectX12_GLFW();
    }

    void WindowController_DirectX12_GLFW::clearData_DirectX12_GLFW()
    {
        if (!m_Windows.isEmpty())
        {
            for (auto& window : m_Windows.values())
            {
                clearWindowDataInternal(&window);
            }
            m_Windows.clear();
        }
    }

    WindowData* WindowController_DirectX12_GLFW::createWindowInternal(const window_id windowID, const WindowCreateInfo& createInfo)
    {
        WindowDataType* windowData = &m_Windows.add(windowID);
        if (!createWindowGLFW(windowID, windowData, createInfo.size, createInfo.title))
        {
            JUTILS_LOG(error, JSTR("Failed to create GLFW window {}"), windowID);
            m_Windows.remove(windowID);
            return nullptr;
        }
        windowData->windowHandler = glfwGetWin32Window(windowData->windowGLFW);

        if (!createWindowSwapchain(windowID, windowData))
        {
            destroyWindowInternal(windowID, windowData);
            return nullptr;
        }
        return windowData;
    }
    void WindowController_DirectX12_GLFW::destroyWindowInternal(const window_id windowID, WindowData* windowData)
    {
        clearWindowDataInternal(windowData);

        m_Windows.remove(windowID);
    }
}

#endif

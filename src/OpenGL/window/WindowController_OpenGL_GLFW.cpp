// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_OPENGL) && defined(GLFW_ENABLED)

#include "WindowController_OpenGL_GLFW.h"

namespace JumaRenderEngine
{
    WindowController_OpenGL_GLFW::~WindowController_OpenGL_GLFW()
    {
        clearData_OpenGL_GLFW();
    }

    void WindowController_OpenGL_GLFW::clearData_OpenGL_GLFW()
    {
        if (!m_Windows.isEmpty())
        {
            for (auto& window : m_Windows)
            {
                clearWindowDataInternal(&window.value);
            }
            m_Windows.clear();
        }

        if (m_DefaultWindow != nullptr)
        {
            glfwDestroyWindow(m_DefaultWindow);
            m_DefaultWindow = nullptr;
        }
    }

    WindowData* WindowController_OpenGL_GLFW::createWindowInternal(const window_id windowID, const WindowCreateInfo& createInfo)
    {
        if (m_DefaultWindow == nullptr)
        {
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            m_DefaultWindow = glfwCreateWindow(1, 1, "", nullptr, nullptr);
            glfwMakeContextCurrent(m_DefaultWindow);
            if (!initOpenGL())
            {
                JUTILS_LOG(error, JSTR("Failed to initialize OpenGL"));
                return nullptr;
            }
        }

        WindowDataType* windowData = &m_Windows.add(windowID);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        if (!createWindowGLFW(windowID, windowData, createInfo.size, createInfo.title, m_DefaultWindow))
        {
            m_Windows.remove(windowID);
            return nullptr;
        }

        const bool vsyncEnabled = true;
        const bool shouldEnabledVsync = vsyncEnabled && ((getMainWindowID() == window_id_INVALID) || (getMainWindowID() == windowID));
        const int32 swapInterval = shouldEnabledVsync ? 1 : 0;

        const window_id prevActiveWindowID = getActiveWindowID();
        setActiveWindowID(windowID);
        glfwSwapInterval(swapInterval);
        setActiveWindowID(prevActiveWindowID);
        return windowData;
    }
    void WindowController_OpenGL_GLFW::destroyWindowInternal(const window_id windowID, WindowData* windowData)
    {
        clearWindowDataInternal(windowData);

        m_Windows.remove(windowID);
    }

    bool WindowController_OpenGL_GLFW::setActiveWindowInternal(const window_id windowID)
    {
        const auto* windowData = m_Windows.find(windowID);
        if (windowData != nullptr)
        {
            glfwMakeContextCurrent(windowData->windowGLFW);
        }
        else
        {
            glfwMakeContextCurrent(m_DefaultWindow);
        }
        return true;
    }

    void WindowController_OpenGL_GLFW::onFinishWindowRender(const window_id windowID)
    {
        Super::onFinishWindowRender(windowID);

        const WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
        if (windowData != nullptr)
        {
            glfwSwapBuffers(windowData->windowGLFW);
        }
    }
}

#endif

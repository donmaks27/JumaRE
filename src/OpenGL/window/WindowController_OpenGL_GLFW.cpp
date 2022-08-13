// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_OPENGL) && defined(JUMARE_ENABLE_LIB_GLFW)

#include "WindowController_OpenGL_GLFW.h"

#include <GLFW/glfw3.h>

namespace JumaRenderEngine
{
    WindowController_OpenGL_GLFW::~WindowController_OpenGL_GLFW()
    {
        clearGLFW();
    }

    bool WindowController_OpenGL_GLFW::initWindowController()
    {
        if (!Super::initWindowController())
        {
            return false;
        }
        if (!initGLFW())
        {
            JUTILS_LOG(error, JSTR("Failed to initialize GLFW"));
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 0);
        return true;
    }

    void WindowController_OpenGL_GLFW::clearGLFW()
    {
        for (auto& window : m_Windows)
        {
            clearWindowDataGLFW(window.key, window.value);
        }
        m_Windows.clear();

        if (m_DefaultWindow != nullptr)
        {
            glfwDestroyWindow(m_DefaultWindow);
        }

        terminateGLFW();
    }

    WindowData* WindowController_OpenGL_GLFW::createWindowInternal(const window_id windowID, const WindowInitProperties& properties)
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

        if (m_DefaultWindow == nullptr)
        {
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            m_DefaultWindow = glfwCreateWindow(1, 1, "", nullptr, nullptr);
            glfwMakeContextCurrent(m_DefaultWindow);
            initOpenGL();
        }

        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

        WindowData_OpenGL_GLFW* windowData = &m_Windows.add(windowID);
        if (!createWindowGLFW(windowData, properties.size, properties.title, m_DefaultWindow))
        {
            JUTILS_LOG(error, JSTR("Failed to create window {}"), windowID);
            return nullptr;
        }

        const window_id prevActiveWindowID = getActiveWindowID();
        setActiveWindowID(windowID);
        glfwSwapInterval(1);
        setActiveWindowID(prevActiveWindowID);
        return windowData;
    }

    bool WindowController_OpenGL_GLFW::setActiveWindowInternal(const window_id windowID)
    {
        const WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
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

    void WindowController_OpenGL_GLFW::destroyWindow(const window_id windowID)
    {
        WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
        if (windowData != nullptr)
        {
            clearWindowDataGLFW(windowID, *windowData);
            m_Windows.remove(windowID);
        }
    }
    void WindowController_OpenGL_GLFW::clearWindowDataGLFW(const window_id windowID, WindowData_OpenGL_GLFW& windowData)
    {
        clearWindowData(windowID, windowData);
        destroyWindowGLFW(&windowData);
    }

    bool WindowController_OpenGL_GLFW::shouldCloseWindow(const window_id windowID) const
    {
        const WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Can't find window {}"), windowID);
            return false;
        }
        return shouldCloseWindowGLFW(windowData);
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
    void WindowController_OpenGL_GLFW::updateWindows()
    {
        pushWindowEventsGLFW();
        Super::updateWindows();
    }
}

#endif

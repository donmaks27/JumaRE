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
            for (auto& window : m_Windows.values())
            {
                clearWindowDataInternal(&window);
            }
            m_Windows.clear();
        }
        m_AsyncAssetTaskQueueWorkerContexts.clear();

        if (m_DefaultWindow != nullptr)
        {
            glfwDestroyWindow(m_DefaultWindow);
            m_DefaultWindow = nullptr;
        }
    }

    WindowData* WindowController_OpenGL_GLFW::createWindowInternal(const window_id windowID, const WindowCreateInfo& createInfo)
    {
        bool defaultWindowCreated = false;
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
            defaultWindowCreated = true;
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
        if (!defaultWindowCreated)
        {
	        setActiveWindowID(prevActiveWindowID);
        }
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

    bool WindowController_OpenGL_GLFW::createContextForAsyncAssetTaskQueueWorker(const int32 workerIndex)
    {
        if (workerIndex < 0)
        {
            JUTILS_LOG(error, JSTR("Invalid worker index {}"), workerIndex);
            return false;
        }
        if (m_DefaultWindow == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to create OpenGL asset loading contexts: empty default window"));
	        return false;
        }
        if (!m_AsyncAssetTaskQueueWorkerContexts.isValidIndex(workerIndex))
        {
            m_AsyncAssetTaskQueueWorkerContexts.resize(workerIndex + 1, nullptr);
        }

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        GLFWwindow* window = glfwCreateWindow(1, 1, "", nullptr, m_DefaultWindow);
        if (window == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to create OpenGL asset loading contexts: failed to create window"));
            return false;
        }

        m_AsyncAssetTaskQueueWorkerContexts[workerIndex] = window;
        return true;
    }
    bool WindowController_OpenGL_GLFW::initAsyncAssetTaskQueueWorkerThread(const int32 workerIndex)
    {
        if (!m_AsyncAssetTaskQueueWorkerContexts.isValidIndex(workerIndex))
        {
            return false;
        }
        GLFWwindow* window = m_AsyncAssetTaskQueueWorkerContexts[workerIndex];
        if (window == nullptr)
        {
            return false;
        }

        glfwMakeContextCurrent(window);
        if (!initOpenGL())
        {
            glfwMakeContextCurrent(nullptr);
            return false;
        }
        return true;
    }
    void WindowController_OpenGL_GLFW::clearAsyncAssetTaskQueueWorkerThread(const int32 workerIndex)
    {
        glfwMakeContextCurrent(nullptr);
    }
    void WindowController_OpenGL_GLFW::destroyContextForAsyncAssetTaskQueueWorker(const int32 workerIndex)
    {
        if (!m_AsyncAssetTaskQueueWorkerContexts.isValidIndex(workerIndex))
        {
            JUTILS_LOG(warning, JSTR("Can't find context for worker {}"), workerIndex);
            return;
        }
        glfwDestroyWindow(m_AsyncAssetTaskQueueWorkerContexts[workerIndex]);
        m_AsyncAssetTaskQueueWorkerContexts[workerIndex] = nullptr;
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

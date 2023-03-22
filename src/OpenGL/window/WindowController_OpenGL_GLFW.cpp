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

        for (const auto& window : m_AssetLoadingWindows)
        {
	        if (window != nullptr)
	        {
		        glfwDestroyWindow(window);
	        }
        }
        m_AssetLoadingWindows.clear();

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

    bool WindowController_OpenGL_GLFW::createContextsForAsyncTaskThreads(const int32 count)
    {
        if (count <= 0)
        {
            JUTILS_LOG(error, JSTR("Failed to create OpenGL asset loading contexts: count is 0"));
	        return false;
        }
        if (m_DefaultWindow == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to create OpenGL asset loading contexts: empty default window"));
	        return false;
        }

        m_AssetLoadingWindows.resize(count);
        for (auto& window : m_AssetLoadingWindows)
        {
	        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            window = glfwCreateWindow(1, 1, "", nullptr, m_DefaultWindow);
            if (window == nullptr)
            {
                JUTILS_LOG(error, JSTR("Failed to create OpenGL asset loading contexts: failed to create window"));
	            return false;
            }
        }
        return true;
    }
    bool WindowController_OpenGL_GLFW::initAsyncTaskThread()
    {
        if (m_DefaultWindow == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Failed to init OpenGL asset loading thread: empty default window"));
	        return false;
        }

        const std::thread::id threadID = std::this_thread::get_id();
        int32 unusedContextIndex = -1;
	    {
		    std::lock_guard lock(m_AssetLoadingThreadsMutex);
        	if (m_AssetLoadingThreads.contains(threadID))
        	{
        		JUTILS_LOG(warning, JSTR("Failed to init OpenGL asset loading thread: thread already in use"));
        		return false;
        	}
        	for (int32 index = 0; index < m_AssetLoadingWindows.getSize(); index++)
        	{
        		bool contextFound = false;
        		for (const auto& loadingThread : m_AssetLoadingThreads)
        		{
        			if (loadingThread.value == index)
        			{
        				contextFound = true;
        				break;
        			}
        		}
        		if (!contextFound)
        		{
        			unusedContextIndex = index;
        			break;
        		}
        	}
        	if (unusedContextIndex == -1)
        	{
        		JUTILS_LOG(warning, JSTR("Failed to init OpenGL asset loading thread: can't find any free context"));
        		return false;
        	}
        	m_AssetLoadingThreads.add(threadID, unusedContextIndex);
	    }

        GLFWwindow* window = m_AssetLoadingWindows[unusedContextIndex];
        glfwMakeContextCurrent(window);
        if (!initOpenGL())
        {
            JUTILS_LOG(warning, JSTR("Failed to init OpenGL asset loading thread: failed to initialize OpenGL"));
            return false;
        }
        return true;
    }
    void WindowController_OpenGL_GLFW::clearAsyncTaskThread()
    {
        const std::thread::id threadID = std::this_thread::get_id();
        std::lock_guard lock(m_AssetLoadingThreadsMutex);
        m_AssetLoadingThreads.remove(threadID);
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

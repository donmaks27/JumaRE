// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_LIB_GLFW)

#include "WindowController_GLFW.h"

#include <GLFW/glfw3.h>

namespace JumaRenderEngine
{
    WindowController_GLFW* GlobalWindowController = nullptr;

    bool WindowController_GLFW::GLFW_init(const RenderAPI renderAPI)
    {
        if (glfwInit() == GLFW_FALSE)
        {
#ifndef JUTILS_LOG_DISABLED
            const char* errorStr = nullptr;
            glfwGetError(&errorStr);
            JUTILS_LOG(error, JSTR("Failed to initialize GLFW lib: {}"), errorStr);
#endif
            return false;
        }

        int monitorsCount = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorsCount);
        for (int index = 0; index < monitorsCount; index++)
        {
            GLFW_onMonitorConnected(monitors[index]);
        }

        GlobalWindowController = this;
        glfwSetErrorCallback(WindowController_GLFW::GLFW_ErrorCallback);
        glfwSetMonitorCallback(WindowController_GLFW::GLFW_MonitorCallback);
        switch (renderAPI)
        {
        case RenderAPI::OpenGL: 
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            break;
        case RenderAPI::Vulkan:
        case RenderAPI::DirectX11:
        case RenderAPI::DirectX12:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
        default: ;
        }
        return true;
    }
    void WindowController_GLFW::GLFW_terminate()
    {
        GlobalWindowController->m_Monitors.clear();

        GlobalWindowController = nullptr;
        glfwTerminate();
    }

    void WindowController_GLFW::GLFW_ErrorCallback(const int errorCode, const char* errorMessage)
    {
        JUTILS_LOG(error, JSTR("GLFW error. Code: {:#x}. {}"), errorCode, errorMessage);
    }

    void WindowController_GLFW::GLFW_MonitorCallback(GLFWmonitor* monitor, const int eventCode)
    {
        switch (eventCode)
        {
        case GLFW_CONNECTED:
            GlobalWindowController->GLFW_onMonitorConnected(monitor);
            break;
        case GLFW_DISCONNECTED:
            GlobalWindowController->GLFW_onMonitorDisconnected(monitor);
            break;
        default: ;
        }
    }
    void WindowController_GLFW::GLFW_onMonitorConnected(GLFWmonitor* monitor)
    {
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

        const monitor_id monitorID = m_MonitorIDs.getUID();
        MonitorData_GLFW& monitorData = m_Monitors.add(monitorID);
        monitorData.monitorID = monitorID;
        monitorData.name = glfwGetMonitorName(monitor);
        monitorData.size = { static_cast<uint32>(videoMode->width), static_cast<uint32>(videoMode->height) };
        monitorData.refreshRate = videoMode->refreshRate;
        monitorData.monitorGLFW = monitor;
        glfwSetMonitorUserPointer(monitor, &monitorData);

        JUTILS_LOG(info, JSTR("Monitor {} connected: {}. Size {{ {}; {} }}"), monitorID, monitorData.name, monitorData.size.x, monitorData.size.y);
    }
    void WindowController_GLFW::GLFW_onMonitorDisconnected(GLFWmonitor* monitor)
    {
        const MonitorData_GLFW* monitorData = static_cast<MonitorData_GLFW*>(glfwGetMonitorUserPointer(monitor));
        JUTILS_LOG(info, JSTR("Monitor {} disconnected: {}"), monitorData->monitorID, monitorData->name);

        // TODO: Set window mode to normal

        m_Monitors.remove(monitorData->monitorID);
    }

    monitor_id WindowController_GLFW::GLFW_getPrimaryMonitorID() const
    {
        const GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        for (const auto& monitorData : m_Monitors)
        {
            if (monitorData.value.monitorGLFW == monitor)
            {
                return monitorData.key;
            }
        }
        return monitor_id_INVALID;
    }

    bool WindowController_GLFW::GLFW_createWindow(WindowData_GLFW* windowDataGLFW, WindowData* windowData, const math::uvector2& size, const jstring& title, GLFWwindow* sharedWindow)
    {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        GLFWwindow* window = glfwCreateWindow(
            static_cast<int>(size.x), static_cast<int>(size.y), *title, nullptr, sharedWindow
        );
        if (window == nullptr)
        {
            JUTILS_LOG(error, JSTR("Failed to create GLFW window"));
            return false;
        }

        windowDataGLFW->windowGLFW = window;
        glfwSetWindowUserPointer(window, windowData);
        glfwSetFramebufferSizeCallback(window, WindowController_GLFW::GLFW_FramebufferResizeCallback);
        glfwSetWindowIconifyCallback(window, WindowController_GLFW::GLFW_WindowMinimizationCallback);
        return true;
    }
    void WindowController_GLFW::GLFW_destroyWindow(WindowData_GLFW* windowData)
    {
        glfwDestroyWindow(windowData->windowGLFW);
        windowData->windowGLFW = nullptr;
    }

    bool WindowController_GLFW::GLFW_shouldCloseWindow(const WindowData_GLFW* windowData) const
    {
        return glfwWindowShouldClose(windowData->windowGLFW) == GLFW_TRUE;
    }
    void WindowController_GLFW::GLFW_pushWindowEvents()
    {
        glfwPollEvents();
    }

    void WindowController_GLFW::GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, const int width, const int height)
    {
        GlobalWindowController->GLFW_onWindowResized(
            static_cast<WindowData*>(glfwGetWindowUserPointer(windowGLFW)), { static_cast<uint32>(width), static_cast<uint32>(height) }
        );
    }

    void WindowController_GLFW::GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, const int minimized)
    {
        GlobalWindowController->GLFW_onWindowMinimizationChanged(
            static_cast<WindowData*>(glfwGetWindowUserPointer(windowGLFW)), minimized == GLFW_TRUE
        );
    }

    void WindowController_GLFW::GLFW_setWindowTitle(const WindowData_GLFW* windowData, const jstring& title)
    {
        glfwSetWindowTitle(windowData->windowGLFW, *title);
    }

    bool WindowController_GLFW::GLFW_setWindowMode(WindowData_GLFW* windowDataGLFW, WindowData* windowData, const WindowMode mode, const monitor_id monitorID)
    {
        if (mode == WindowMode::Normal)
        {
            glfwSetWindowMonitor(
                windowDataGLFW->windowGLFW, nullptr, 
                windowDataGLFW->originalWindowPosition.x, windowDataGLFW->originalWindowPosition.y, 
                windowData->desiredSize.x, windowData->desiredSize.y, GLFW_DONT_CARE
            );
            GLFW_onWindowModeChanged(windowData, mode);
            return true;
        }

        const MonitorData_GLFW* monitorData = GLFW_findMonitorData(monitorID);
        if (monitorData == nullptr)
        {
            JUTILS_LOG(warning, JSTR("Can't find monitor {}"), monitorID);
            return false;
        }

        if (windowData->mode == WindowMode::Normal)
        {
            int width, height;
            glfwGetWindowPos(windowDataGLFW->windowGLFW, &width, &height);
            windowDataGLFW->originalWindowPosition.x = width;
            windowDataGLFW->originalWindowPosition.y = height;
        }
        if (mode == WindowMode::Fullscreen)
        {
            glfwSetWindowMonitor(
                windowDataGLFW->windowGLFW, monitorData->monitorGLFW, 0, 0, 
                windowData->desiredSize.x, windowData->desiredSize.y, GLFW_DONT_CARE
            );
        }
        else
        {
            glfwSetWindowMonitor(
                windowDataGLFW->windowGLFW, monitorData->monitorGLFW, 0, 0, 
                monitorData->size.x, monitorData->size.y, monitorData->refreshRate
            );
        }
        GLFW_onWindowModeChanged(windowData, mode);
        return true;
    }
}

#endif

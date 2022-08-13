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
        GlobalWindowController = nullptr;
        glfwTerminate();
    }

    void WindowController_GLFW::GLFW_ErrorCallback(const int errorCode, const char* errorMessage)
    {
        JUTILS_LOG(error, JSTR("GLFW error. Code: {:#x}. {}"), errorCode, errorMessage);
    }

    void WindowController_GLFW::GLFW_MonitorCallback(GLFWmonitor* monitor, const int event)
    {
        // TODO: Track monitors; Set window mode to normal
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
}

#endif

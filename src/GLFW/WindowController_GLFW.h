// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_LIB_GLFW)

#include "../../include/JumaRE/window/WindowController.h"

#include <GLFW/glfw3.h>

#include "../../include/JumaRE/RenderAPI.h"

namespace JumaRenderEngine
{
    template<typename BaseWindowData, TEMPLATE_ENABLE(is_base_and_not_same<WindowData, BaseWindowData>)>
    struct WindowData_GLFW : BaseWindowData
    {
        GLFWwindow* windowGLFW = nullptr;

        math::ivector2 originalWindowPosition;
        math::ivector2 originalMonitorSize;
        int32 originalMonitorRedreshRate = 0;
    };

    template<typename BaseWindowController, TEMPLATE_ENABLE(is_base_and_not_same<WindowController, BaseWindowController>)>
    class WindowController_GLFW : public BaseWindowController
    {
        using Super = BaseWindowController;

    public:
        WindowController_GLFW() = default;
        virtual ~WindowController_GLFW() override { clearData_GLFW(); }

        using WindowDataType = WindowData_GLFW<typename Super::WindowDataType>;

        virtual bool shouldCloseWindow(window_id windowID) const override;

        virtual void updateWindows() override;

    protected:

        virtual bool initWindowController() override;

        bool createWindowGLFW(window_id windowID, WindowDataType* windowData, const math::uvector2& size, const jstring& title, 
            GLFWwindow* sharedWindow = nullptr);
        virtual void destroyWindowInternal(window_id windowID, WindowData* windowData) override;

        virtual bool setMainWindowModeInternal(WindowMode windowMode) override;

    private:

        static inline WindowController_GLFW* GlobalWindowController = nullptr;


        void clearData_GLFW();

        static void GLFW_ErrorCallback(const int errorCode, const char* errorMessage) { JUTILS_LOG(error, JSTR("GLFW error. Code: {:#x}. {}"), errorCode, errorMessage); }
        static void GLFW_MonitorCallback(GLFWmonitor* monitor, int eventCode);
        void onMonitorDisconnected(GLFWmonitor* monitor);

        static void GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height);
        static void GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, int minimized);
    };

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    bool WindowController_GLFW<BaseWindowController, Condition>::initWindowController()
    {
        if (!Super::initWindowController())
        {
            return false;
        }

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
        if (Super::API == RenderAPI::OpenGL)
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_SAMPLES, 0);
        }
        else
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }
        return true;
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::clearData_GLFW()
    {
        GlobalWindowController = nullptr;
        glfwTerminate();
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_MonitorCallback(GLFWmonitor* monitor, const int eventCode)
    {
        if (eventCode == GLFW_DISCONNECTED)
        {
            GlobalWindowController->onMonitorDisconnected(monitor);
        }
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::onMonitorDisconnected(GLFWmonitor* monitor)
    {
        const WindowData* windowData = static_cast<WindowData*>(glfwGetMonitorUserPointer(monitor));
        if ((windowData != nullptr) && (windowData->windowID == this->getMainWindowID()))
        {
            this->updateMainWindowMode(WindowMode::Normal);
        }
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    bool WindowController_GLFW<BaseWindowController, Condition>::createWindowGLFW(const window_id windowID, WindowDataType* windowData, 
        const math::uvector2& size, const jstring& title, GLFWwindow* sharedWindow)
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

        windowData->windowGLFW = window;
        glfwSetWindowUserPointer(window, windowData);
        glfwSetFramebufferSizeCallback(window, WindowController_GLFW::GLFW_FramebufferResizeCallback);
        glfwSetWindowIconifyCallback(window, WindowController_GLFW::GLFW_WindowMinimizationCallback);
        return true;
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::destroyWindowInternal(const window_id windowID, WindowData* windowData)
    {
        Super::destroyWindowInternal(windowID, windowData);

        WindowDataType* windowDataGLFW = reinterpret_cast<WindowDataType*>(windowData);
        if (windowDataGLFW->windowGLFW != nullptr)
        {
            glfwDestroyWindow(windowDataGLFW->windowGLFW);
            windowDataGLFW->windowGLFW = nullptr;
        }
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, const int width, 
        const int height)
    {
        const WindowData* windowData = static_cast<WindowData*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            GlobalWindowController->updateWindowSize(windowData->windowID, math::uvector2(static_cast<uint32>(width), static_cast<uint32>(height)));
        }
    }
    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, const int minimized)
    {
        const WindowData* windowData = static_cast<WindowData*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            GlobalWindowController->updateWindowMinimization(windowData->windowID, minimized == GLFW_TRUE);
        }
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    bool WindowController_GLFW<BaseWindowController, Condition>::shouldCloseWindow(const window_id windowID) const
    {
        const WindowDataType* windowData = this->template findWindowData<WindowDataType>(windowID);
        return (windowData != nullptr) && (glfwWindowShouldClose(windowData->windowGLFW) == GLFW_TRUE);
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    bool WindowController_GLFW<BaseWindowController, Condition>::setMainWindowModeInternal(const WindowMode windowMode)
    {
        WindowDataType* windowData = this->template getWindowData<WindowDataType>(this->getMainWindowID());
        if (windowData == nullptr)
        {
            return false;
        }
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if (monitor == nullptr)
        {
            return false;
        }

        if (this->getMainWindowMode() == WindowMode::Normal)
        {
            const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
            if (videoMode == nullptr)
            {
                return false;
            }

            glfwGetWindowPos(windowData->windowGLFW, &windowData->originalWindowPosition.x, &windowData->originalWindowPosition.y);
            windowData->originalMonitorSize = { videoMode->width, videoMode->height };
            windowData->originalMonitorRedreshRate = videoMode->refreshRate;

            glfwSetMonitorUserPointer(monitor, windowData);
        }

        switch (windowMode)
        {
        case WindowMode::Normal: 
            glfwSetWindowMonitor(windowData->windowGLFW, nullptr, 
                windowData->originalWindowPosition.x, windowData->originalWindowPosition.y, 
                windowData->desiredSize.x, windowData->desiredSize.y, GLFW_DONT_CARE
            );
            glfwSetMonitorUserPointer(monitor, nullptr);
            break;
        case WindowMode::Fullscreen: 
            glfwSetWindowMonitor(windowData->windowGLFW, monitor, 0, 0, 
                windowData->desiredSize.x, windowData->desiredSize.y, GLFW_DONT_CARE
            );
            break;
        case WindowMode::WindowedFullscreen: 
            glfwSetWindowMonitor(windowData->windowGLFW, monitor, 0, 0, 
                windowData->originalMonitorSize.x, windowData->originalMonitorSize.y, windowData->originalMonitorRedreshRate
            );
            break;
        default: ;
        }
        return true;
    }

    template<typename BaseWindowController, TEMPLATE_ENABLE_IMPL(is_base_and_not_same<WindowController, BaseWindowController>) Condition>
    void WindowController_GLFW<BaseWindowController, Condition>::updateWindows()
    {
        glfwPollEvents();
        Super::updateWindows();
    }
}

#endif

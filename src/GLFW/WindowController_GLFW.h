// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_LIB_GLFW)

#include "../../include/JumaRE/window/WindowController.h"

struct GLFWmonitor;
struct GLFWwindow;

namespace JumaRenderEngine
{
    struct WindowData_GLFW
    {
        GLFWwindow* windowGLFW = nullptr;
    };

    class WindowController_GLFW
    {
    public:
        WindowController_GLFW() = default;
        virtual ~WindowController_GLFW() = default;

    protected:

        bool initGLFW();
        void terminateGLFW();

        bool createWindowGLFW(WindowData_GLFW* windowDataGLFW, WindowData* windowData, const math::uvector2& size, const jstring& title, 
            GLFWwindow* sharedWindow);
        template<typename T>
        bool createWindowGLFW(T* windowData, const math::uvector2& size, const jstring& title, GLFWwindow* sharedWindow = nullptr)
        {
            return this->createWindowGLFW(windowData, windowData, size, title, sharedWindow);
        }
        void destroyWindowGLFW(WindowData_GLFW* windowData);

        bool shouldCloseWindowGLFW(const WindowData_GLFW* windowData) const;
        void pushWindowEventsGLFW();

        virtual void onWindowResizedGLFW(WindowData* windowData, const math::uvector2& size) = 0;
        virtual void onWindowMinimizationChangedGLFW(WindowData* windowData, bool minimized) = 0;

        void setWindowTitleGLFW(const WindowData* windowData, const jstring& title);

    private:

        static void GLFW_ErrorCallback(int errorCode, const char* errorMessage);
        static void GLFW_MonitorCallback(GLFWmonitor* monitor, int event);

        static void GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height);
        static void GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, int minimized);
    };
}

#endif

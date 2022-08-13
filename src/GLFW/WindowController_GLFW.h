// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_LIB_GLFW)

#include "../../include/JumaRE/window/WindowController.h"

#include "../../include/JumaRE/RenderAPI.h"

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

        bool GLFW_init(RenderAPI renderAPI);
        void GLFW_terminate();

        bool GLFW_createWindow(WindowData_GLFW* windowDataGLFW, WindowData* windowData, const math::uvector2& size, const jstring& title, 
            GLFWwindow* sharedWindow);
        template<typename T>
        bool GLFW_createWindow(T* windowData, const math::uvector2& size, const jstring& title, GLFWwindow* sharedWindow = nullptr)
        {
            return this->GLFW_createWindow(windowData, windowData, size, title, sharedWindow);
        }
        void GLFW_destroyWindow(WindowData_GLFW* windowData);

        bool GLFW_shouldCloseWindow(const WindowData_GLFW* windowData) const;
        void GLFW_pushWindowEvents();

        virtual void GLFW_onWindowResized(WindowData* windowData, const math::uvector2& size) = 0;
        virtual void GLFW_onWindowMinimizationChanged(WindowData* windowData, bool minimized) = 0;

        void GLFW_setWindowTitle(const WindowData_GLFW* windowData, const jstring& title);

    private:

        static void GLFW_ErrorCallback(int errorCode, const char* errorMessage);
        static void GLFW_MonitorCallback(GLFWmonitor* monitor, int event);

        static void GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height);
        static void GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, int minimized);
    };
}

#endif

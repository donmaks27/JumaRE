// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_LIB_GLFW)

#include "../../include/JumaRE/window/WindowController.h"

#include "../../include/JumaRE/RenderAPI.h"

struct GLFWmonitor;
struct GLFWwindow;

namespace JumaRenderEngine
{
    struct MonitorData_GLFW : MonitorData
    {
        uint32 refreshRate = 0;

        GLFWmonitor* monitorGLFW = nullptr;
    };

    struct WindowData_GLFW
    {
        GLFWwindow* windowGLFW = nullptr;

        math::uvector2 originalWindowPosition = { 0, 0 };
    };

    class WindowController_GLFW
    {
    public:
        WindowController_GLFW() = default;
        virtual ~WindowController_GLFW() = default;

    protected:

        bool GLFW_init(RenderAPI renderAPI);
        void GLFW_terminate();

        monitor_id GLFW_getPrimaryMonitorID() const;
        const MonitorData_GLFW* GLFW_findMonitorData(const monitor_id monitorID) const { return m_Monitors.find(monitorID); }
        jarray<monitor_id> GLFW_getMonitorIDs() const { return m_Monitors.getKeys(); }

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
        virtual void GLFW_onWindowModeChanged(WindowData* windowData, WindowMode mode) = 0;

        void GLFW_setWindowTitle(const WindowData_GLFW* windowData, const jstring& title);
        bool GLFW_setWindowMode(WindowData_GLFW* windowDataGLFW, WindowData* windowData, WindowMode mode, monitor_id monitorID);
        template<typename T>
        bool GLFW_setWindowMode(T* windowData, const WindowMode mode, const monitor_id monitorID) { return this->GLFW_setWindowMode(windowData, windowData, mode, monitorID); }

    private:

        juid<monitor_id> m_MonitorIDs;
        jmap<monitor_id, MonitorData_GLFW> m_Monitors;


        static void GLFW_ErrorCallback(int errorCode, const char* errorMessage);

        static void GLFW_MonitorCallback(GLFWmonitor* monitor, int eventCode);
        void GLFW_onMonitorConnected(GLFWmonitor* monitor);
        void GLFW_onMonitorDisconnected(GLFWmonitor* monitor);

        static void GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height);
        static void GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, int minimized);
    };
}

#endif

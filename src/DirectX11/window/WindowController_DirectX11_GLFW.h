﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11) && defined(JUMARE_ENABLE_LIB_GLFW)

#include "WindowController_DirectX11.h"

struct GLFWwindow;

namespace JumaRenderEngine
{
    class WindowController_DirectX11_GLFW;

    struct WindowData_DirectX11_GLFW : WindowData_DirectX11
    {
        GLFWwindow* windowGLFW = nullptr;

        WindowController_DirectX11_GLFW* windowController = nullptr;
    };

    class WindowController_DirectX11_GLFW final : public WindowController_DirectX11
    {
        using Super = WindowController_DirectX11;

    public:
        WindowController_DirectX11_GLFW() = default;
        virtual ~WindowController_DirectX11_GLFW() override;

        virtual void destroyWindow(window_id windowID) override;

        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }
        virtual jarray<window_id> getWindowIDs() const override { return m_Windows.getKeys(); }

        virtual bool shouldCloseWindow(window_id windowID) const override;

        virtual void updateWindows() override;

    protected:

        virtual bool initWindowController() override;

        virtual WindowData* createWindowInternal(window_id windowID, const WindowInitProperties& properties) override;

        virtual WindowData* getWindowData(const window_id windowID) override { return m_Windows.find(windowID); }

        virtual void setWindowTitleInternal(WindowData* windowData, const jstring& title) override;

    private:

        jmap<window_id, WindowData_DirectX11_GLFW> m_Windows;

        
        static void GLFW_ErrorCallback(int errorCode, const char* errorMessage);
        static void GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height);
        static void GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, int minimized);

        void clearGLFW();

        void clearWindowDataGLFW(window_id windowID, WindowData_DirectX11_GLFW& windowData);
    };
}

#endif
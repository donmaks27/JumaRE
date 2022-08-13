// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL) && defined(JUMARE_ENABLE_LIB_GLFW)

#include "WindowController_OpenGL.h"
#include "../../GLFW/WindowController_GLFW.h"

namespace JumaRenderEngine
{
    struct WindowData_OpenGL_GLFW : WindowData_OpenGL, WindowData_GLFW
    {
    };

    class WindowController_OpenGL_GLFW final : public WindowController_OpenGL, protected WindowController_GLFW
    {
        using Super = WindowController_OpenGL;

    public:
        WindowController_OpenGL_GLFW() = default;
        virtual ~WindowController_OpenGL_GLFW() override;

        virtual void destroyWindow(window_id windowID) override;

        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }
        virtual jarray<window_id> getWindowIDs() const override { return m_Windows.getKeys(); }

        virtual bool shouldCloseWindow(window_id windowID) const override;

        virtual void onFinishWindowRender(window_id windowID) override;
        virtual void updateWindows() override;

    protected:

        virtual bool initWindowController() override;

        virtual WindowData* createWindowInternal(window_id windowID, const WindowInitProperties& properties) override;
        
        virtual WindowData* getWindowData(const window_id windowID) override { return m_Windows.find(windowID); }

        virtual bool setActiveWindowInternal(window_id windowID) override;

        virtual void onWindowResizedGLFW(WindowData* windowData, const math::uvector2& size) override { updateWindowSize(windowData->windowID, size); }
        virtual void onWindowMinimizationChangedGLFW(WindowData* windowData, const bool minimized) override { updateWindowMinimization(windowData->windowID, minimized); }

        virtual void setWindowTitleInternal(WindowData* windowData, const jstring& title) override { setWindowTitleGLFW(windowData, title); }

    private:

        GLFWwindow* m_DefaultWindow = nullptr;
        jmap<window_id, WindowData_OpenGL_GLFW> m_Windows;


        void clearGLFW();

        void clearWindowDataGLFW(window_id windowID, WindowData_OpenGL_GLFW& windowData);
    };
}

#endif

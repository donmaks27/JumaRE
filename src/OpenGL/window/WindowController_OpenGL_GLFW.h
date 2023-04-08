// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL) && defined(GLFW_ENABLED)

#include "WindowController_OpenGL.h"
#include "../../GLFW/WindowController_GLFW.h"

#include <mutex>
#include <thread>
#include <jutils/jmap.h>

namespace JumaRenderEngine
{
    using WindowData_OpenGL_GLFW = WindowData_GLFW<WindowData_OpenGL>;

    class WindowController_OpenGL_GLFW final : public WindowController_GLFW<WindowController_OpenGL>
    {
        using Super = WindowController_GLFW<WindowController_OpenGL>;

    public:
        WindowController_OpenGL_GLFW() = default;
        virtual ~WindowController_OpenGL_GLFW() override;

        using WindowDataType = WindowData_OpenGL_GLFW;


        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }

        virtual void onFinishWindowRender(window_id windowID) override;

    protected:

        virtual WindowData* createWindowInternal(window_id windowID, const WindowCreateInfo& createInfo) override;
        virtual void destroyWindowInternal(window_id windowID, WindowData* windowData) override;

        virtual WindowData* getWindowData(const window_id windowID) override { return m_Windows.find(windowID); }

        virtual bool setActiveWindowInternal(window_id windowID) override;

        virtual bool createContextForAsyncAssetTaskQueueWorker(int32 workerIndex) override;
        virtual bool initAsyncAssetTaskQueueWorkerThread(int32 workerIndex) override;
        virtual void clearAsyncAssetTaskQueueWorkerThread(int32 workerIndex) override;
        virtual void destroyContextForAsyncAssetTaskQueueWorker(int32 workerIndex) override;

    private:

        GLFWwindow* m_DefaultWindow = nullptr;
        jmap<window_id, WindowData_OpenGL_GLFW> m_Windows;

        jarray<GLFWwindow*> m_AsyncAssetTaskQueueWorkerContexts;


        void clearData_OpenGL_GLFW();
    };
}

#endif

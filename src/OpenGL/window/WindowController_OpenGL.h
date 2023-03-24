// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL)

#include "../../../include/JumaRE/window/WindowController.h"

#include "../../../include/JumaRE/RenderAPI.h"

namespace JumaRenderEngine
{
    class RenderEngine_OpenGL;

    struct WindowData_OpenGL : WindowData
    {
    };

    class WindowController_OpenGL : public WindowController
    {
        using Super = WindowController;

        friend RenderEngine_OpenGL;

    public:
        WindowController_OpenGL() = default;
        virtual ~WindowController_OpenGL() override;

        using WindowDataType = WindowData_OpenGL;


        window_id getActiveWindowID() const { return m_ActiveWindowID; }
        void setActiveWindowID(window_id windowID);

    protected:

        static constexpr RenderAPI API = RenderAPI::OpenGL;


        bool initOpenGL();

        virtual bool setActiveWindowInternal(window_id windowID) = 0;

        virtual bool createContextForAsyncAssetTaskQueueWorker(int32 workerIndex) = 0;
        virtual bool initAsyncAssetTaskQueueWorkerThread(int32 workerIndex) = 0;
        virtual void clearAsyncAssetTaskQueueWorkerThread(int32 workerIndex) = 0;
        virtual void destroyContextForAsyncAssetTaskQueueWorker(int32 workerIndex) = 0;

    private:

        window_id m_ActiveWindowID = window_id_INVALID;


        void clearData_OpenGL();
    };
}

#endif

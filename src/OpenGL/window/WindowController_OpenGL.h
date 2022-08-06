// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_OPENGL)

#include "../../../include/JumaRE/window/WindowController.h"

namespace JumaRenderEngine
{
    struct WindowData_OpenGL : WindowData
    {
    };

    class WindowController_OpenGL : public WindowController
    {
        using Super = WindowController;

    public:
        WindowController_OpenGL() = default;
        virtual ~WindowController_OpenGL() override;

        window_id getActiveWindowID() const { return m_ActiveWindowID; }
        void setActiveWindowID(window_id windowID);

    protected:

        bool initOpenGL();

        virtual bool setActiveWindowInternal(window_id windowID) = 0;

    private:

        window_id m_ActiveWindowID = window_id_INVALID;


        void clearOpenGL();
    };
}

#endif

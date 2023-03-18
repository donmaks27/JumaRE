// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_OPENGL)

#include "WindowController_OpenGL.h"

#include <GL/glew.h>

namespace JumaRenderEngine
{
    WindowController_OpenGL::~WindowController_OpenGL()
    {
        clearData_OpenGL();
    }

    void WindowController_OpenGL::clearData_OpenGL()
    {
        m_ActiveWindowID = window_id_INVALID;
    }

    bool WindowController_OpenGL::initOpenGL()
    {
        const GLenum glewInitResult = glewInit();
        if (glewInitResult != GLEW_OK)
        {
            JUTILS_LOG(error, reinterpret_cast<const char*>(glewGetErrorString(glewInitResult)));
            return false;
        }
        return true;
    }

    void WindowController_OpenGL::setActiveWindowID(window_id windowID)
    {
    	if (windowID == window_id_INVALID)
        {
            windowID = getMainWindowID();
        }
        if (windowID != m_ActiveWindowID)
        {
            if (setActiveWindowInternal(windowID))
            {
                m_ActiveWindowID = windowID;
            }
            else
            {
                JUTILS_LOG(warning, JSTR("Failed to set active window ID {}"), windowID);
            }
        }
    }
}

#endif

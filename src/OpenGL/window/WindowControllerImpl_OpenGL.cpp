// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_OPENGL)

#include "WindowControllerImpl_OpenGL.h"

#include "WindowController_OpenGL_GLFW.h"

namespace JumaRenderEngine
{
    WindowController_OpenGL* CreateWindowController_OpenGL()
    {
#if defined(JUMARE_ENABLE_OPENGL_GLFW)
        return new WindowController_OpenGL_GLFW();
#else
        return nullptr;
#endif
    }
}

#endif

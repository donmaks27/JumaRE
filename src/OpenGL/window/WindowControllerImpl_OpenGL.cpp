// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_INCLUDE_RENDER_API_OPENGL)

#include "WindowControllerImpl_OpenGL.h"

#include "WindowController_OpenGL_GLFW.h"

namespace JumaRenderEngine
{
    WindowController_OpenGL* CreateWindowController_OpenGL()
    {
#if defined(INCLUDE_LIB_GLFW)
        return new WindowController_OpenGL_GLFW();
#else
        return nullptr;
#endif
    }
}

#endif

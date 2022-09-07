// Copyright 2022 Leonov Maksim. All Rights Reserved.

#if defined(JUMARE_ENABLE_OPENGL)

#include "WindowControllerImpl_OpenGL.h"

#include "WindowController_OpenGL_GLFW.h"

namespace JumaRenderEngine
{
    WindowController_OpenGL* CreateWindowController_OpenGL()
    {
#if defined(GLFW_ENABLED)
        return new WindowController_OpenGL_GLFW();
#else
        static_assert(false, "There is no implementation of WindowController_OpenGL class");
#endif
    }
}

#endif

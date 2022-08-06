﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "JumaRE/RenderEngineImpl_OpenGL.h"

#include "RenderEngine_OpenGL.h"

namespace JumaRenderEngine
{
    RenderEngine* CreateRenderEngine_OpenGL()
    {
#if defined(JUMARE_INCLUDE_RENDER_API_OPENGL)
        return new RenderEngine_OpenGL();
#else
        return nullptr;
#endif
    }
}

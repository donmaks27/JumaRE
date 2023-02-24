// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/RenderEngineImpl_OpenGL.h"

#include "RenderEngine_OpenGL.h"

namespace JumaRenderEngine
{
	bool IsSupportRenderAPI_OpenGL()
	{
#if defined(JUMARE_ENABLE_OPENGL)
        return true;
#else
        return false;
#endif
	}

    RenderEngine* CreateRenderEngine_OpenGL()
    {
#if defined(JUMARE_ENABLE_OPENGL)
        return new RenderEngine_OpenGL();
#else
        return nullptr;
#endif
    }
}

// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "RenderEngine.h"

namespace JumaRenderEngine
{
    extern bool IsSupportRenderAPI_OpenGL();
    extern RenderEngine* CreateRenderEngine_OpenGL();
    
    template<>
    inline bool IsSupportRenderAPI<RenderAPI::OpenGL>() { return IsSupportRenderAPI_OpenGL(); }
    template<>
    inline RenderEngine* CreateRenderEngine<RenderAPI::OpenGL>() { return CreateRenderEngine_OpenGL(); }
}

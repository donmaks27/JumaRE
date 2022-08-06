// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "RenderEngineImpl.h"

namespace JumaRenderEngine
{
    extern RenderEngine* CreateRenderEngine_OpenGL();

    template<>
    inline RenderEngine* CreateRenderEngine<RenderAPI::OpenGL>() { return CreateRenderEngine_OpenGL(); }
}

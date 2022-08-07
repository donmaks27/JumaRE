// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/RenderEngineImpl_DirectX11.h"

#include "RenderEngine_DirectX11.h"

namespace JumaRenderEngine
{
    RenderEngine* CreateRenderEngine_DirectX11()
    {
#if defined(JUMARE_ENABLE_DX11)
        return new RenderEngine_DirectX11();
#else
        return nullptr;
#endif
    }
}

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/RenderEngineImpl_DirectX12.h"

#include "RenderEngine_DirectX12.h"

namespace JumaRenderEngine
{
    RenderEngine* CreateRenderEngine_DirectX12()
    {
#if defined(JUMARE_ENABLE_DX12)
        return new RenderEngine_DirectX12();
#else
        return nullptr;
#endif
    }
}

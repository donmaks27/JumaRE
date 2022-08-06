// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "../../include/JumaRE/RenderEngineImpl_Vulkan.h"

#include "RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    RenderEngine* CreateRenderEngine_Vulkan()
    {
#if defined(JUMARE_ENABLE_VULKAN)
        return new RenderEngine_Vulkan();
#else
        return nullptr;
#endif
    }
}

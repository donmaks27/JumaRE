// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#include "JumaRE/RenderEngineImpl_Vulkan.h"

#include "RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
	bool IsSupportRenderAPI_Vulkan()
	{
#if defined(JUMARE_ENABLE_VULKAN)
        return true;
#else
        return false;
#endif
	}

    RenderEngine* CreateRenderEngine_Vulkan()
    {
#if defined(JUMARE_ENABLE_VULKAN)
        return new RenderEngine_Vulkan();
#else
        return nullptr;
#endif
    }
}

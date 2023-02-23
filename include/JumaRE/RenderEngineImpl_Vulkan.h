// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "RenderEngine.h"

namespace JumaRenderEngine
{
    extern bool IsSupportRenderAPI_Vulkan();
    extern RenderEngine* CreateRenderEngine_Vulkan();

    template<>
    inline bool IsSupportRenderAPI<RenderAPI::Vulkan>() { return IsSupportRenderAPI_Vulkan(); }
    template<>
    inline RenderEngine* CreateRenderEngine<RenderAPI::Vulkan>() { return CreateRenderEngine_Vulkan(); }
}

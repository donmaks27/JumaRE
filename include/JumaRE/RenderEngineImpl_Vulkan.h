// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "RenderEngine.h"

namespace JumaRenderEngine
{
    extern RenderEngine* CreateRenderEngine_Vulkan();

    template<>
    inline RenderEngine* CreateRenderEngine<RenderAPI::Vulkan>() { return CreateRenderEngine_Vulkan(); }
}

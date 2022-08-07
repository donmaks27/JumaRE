// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "RenderEngine.h"

namespace JumaRenderEngine
{
    extern RenderEngine* CreateRenderEngine_DirectX12();

    template<>
    inline RenderEngine* CreateRenderEngine<RenderAPI::DirectX12>() { return CreateRenderEngine_DirectX12(); }
}

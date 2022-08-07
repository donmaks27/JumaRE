// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "RenderEngine.h"

namespace JumaRenderEngine
{
    extern RenderEngine* CreateRenderEngine_DirectX11();

    template<>
    inline RenderEngine* CreateRenderEngine<RenderAPI::DirectX11>() { return CreateRenderEngine_DirectX11(); }
}

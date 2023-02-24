// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "RenderEngine.h"

namespace JumaRenderEngine
{
    extern bool IsSupportRenderAPI_DirectX11();
    extern RenderEngine* CreateRenderEngine_DirectX11();
    
    template<>
    inline bool IsSupportRenderAPI<RenderAPI::DirectX11>() { return IsSupportRenderAPI_DirectX11(); }
    template<>
    inline RenderEngine* CreateRenderEngine<RenderAPI::DirectX11>() { return CreateRenderEngine_DirectX11(); }
}

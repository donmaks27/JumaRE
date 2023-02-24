// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "RenderEngine.h"

namespace JumaRenderEngine
{
    extern bool IsSupportRenderAPI_DirectX12();
    extern RenderEngine* CreateRenderEngine_DirectX12();
    
    template<>
    inline bool IsSupportRenderAPI<RenderAPI::DirectX12>() { return IsSupportRenderAPI_DirectX12(); }
    template<>
    inline RenderEngine* CreateRenderEngine<RenderAPI::DirectX12>() { return CreateRenderEngine_DirectX12(); }
}

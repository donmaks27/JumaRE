// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "JumaRE/RenderOptions.h"

namespace JumaRenderEngine
{
    class DirectX12CommandList;

    struct RenderOptions_DirectX12 final : RenderOptions
    {
        DirectX12CommandList* renderCommandList = nullptr;
    };
}

#endif

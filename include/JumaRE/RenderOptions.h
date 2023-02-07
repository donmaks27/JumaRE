// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"

#include "RenderPrimitivesList.h"

namespace JumaRenderEngine
{
    class RenderPipeline;
    class RenderTarget;

    struct RenderOptions
    {
        RenderPipeline* renderPipeline = nullptr;
        RenderTarget* renderTarget = nullptr;

        RenderStageProperties renderStageProperties;
    };
}

// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "core.h"
#include "RenderEngine.h"

namespace JumaRenderEngine
{
    template<RenderAPI API>
    RenderEngine* CreateRenderEngine() { return nullptr; }
}

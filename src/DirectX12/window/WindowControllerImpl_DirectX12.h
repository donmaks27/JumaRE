// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX12)

#include "WindowController_DirectX12.h"

namespace JumaRenderEngine
{
    extern WindowController_DirectX12* CreateWindowController_DirectX12();
}

#endif
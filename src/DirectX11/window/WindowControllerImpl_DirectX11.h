// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_DX11)

#include "WindowController_DirectX11.h"

namespace JumaRenderEngine
{
    extern WindowController_DirectX11* CreateWindowController_DirectX11();
}

#endif
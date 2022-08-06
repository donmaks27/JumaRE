// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "WindowController_Vulkan.h"

namespace JumaRenderEngine
{
    extern WindowController_Vulkan* CreateWindowController_Vulkan();
}

#endif

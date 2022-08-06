// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../../include/JumaRE/core.h"

namespace JumaRenderEngine
{
    enum class VulkanQueueType : uint8
    {
        Graphics, Transfer
    };
}

#endif

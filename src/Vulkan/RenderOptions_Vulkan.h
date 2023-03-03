// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "JumaRE/RenderOptions.h"

namespace JumaRenderEngine
{
    class VulkanCommandBuffer;
    class VulkanRenderPass;

    struct RenderOptions_Vulkan final : RenderOptions
    {
        const VulkanRenderPass* renderPass = nullptr;
        VulkanCommandBuffer* commandBuffer = nullptr;
    };
}

#endif

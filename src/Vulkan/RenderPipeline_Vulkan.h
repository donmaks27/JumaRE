// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "JumaRE/RenderPipeline.h"

#include <vulkan/vulkan_core.h>

namespace JumaRenderEngine
{
    class VulkanSwapchain;
    class VulkanCommandBuffer;

    class RenderPipeline_Vulkan final : public RenderPipeline
    {
        using Super = RenderPipeline;

    public:
        RenderPipeline_Vulkan() = default;
        virtual ~RenderPipeline_Vulkan() override;

        virtual void waitForRenderFinished() override;

    protected:

        virtual bool initInternal() override;

        virtual void renderInternal() override;

        virtual bool onStartRender(RenderOptions* renderOptions) override;
        virtual void onFinishRender(RenderOptions* renderOptions) override;

    private:

        VkFence m_RenderFinishedFence = nullptr;
        VkSemaphore m_RenderFinishedSemaphore = nullptr;

        VulkanCommandBuffer* m_RenderCommandBuffer = nullptr;
        jarray<VulkanSwapchain*> m_Swapchains;
        jarray<VkSemaphore> m_SwapchainImageReadySemaphores;
        

        void clearVulkan();

        void waitForPreviousRenderFinish();
        bool startRecordingRenderCommandBuffer(RenderOptions* renderOptions);
        bool finishRecordingRenderCommandBuffer(RenderOptions* renderOptions);
    };
}

#endif

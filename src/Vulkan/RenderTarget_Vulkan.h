// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../include/JumaRE/RenderTarget.h"

#include <jutils/jarray.h>

#include "vulkanObjects/VulkanFramebufferData.h"

namespace JumaRenderEngine
{
    class VulkanSwapchain;
    class VulkanRenderPass;
    class VulkanImage;

    class RenderTarget_Vulkan final : public RenderTarget
    {
        using Super = RenderTarget;

    public:
        RenderTarget_Vulkan() = default;
        virtual ~RenderTarget_Vulkan() override;

        VulkanImage* getResultImage() const;

        virtual bool onStartRender(RenderOptions* renderOptions) override;
        virtual void onFinishRender(RenderOptions* renderOptions) override;

    protected:

        virtual bool initInternal() override;
        virtual void clearAssetInternal() override;

        virtual bool recreateRenderTarget() override;

    private:

        VulkanRenderPass* m_RenderPass = nullptr;
        jarray<VulkanFramebufferData> m_Framebuffers;
        bool m_FramebuffersValidForRender = false;


        bool initRenderTarget() { return createFramebuffers(); }
        bool initWindowRenderTarget();
        bool createFramebuffers();
        bool createWindowFramebuffers(const VulkanSwapchain* swapchain = nullptr);

        void clearVulkan();
        void clearFramebuffers();

        int32 getRequiredFramebufferIndex() const;

        void onSwapchainRecreated(VulkanSwapchain* swapchain);
    };
}

#endif

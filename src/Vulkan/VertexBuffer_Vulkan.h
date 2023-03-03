// Copyright © 2022-2023 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "JumaRE/vertex/VertexBuffer.h"

namespace JumaRenderEngine
{
    class VulkanBuffer;

    class VertexBuffer_Vulkan final : public VertexBuffer
    {
        using Super = VertexBuffer;

    public:
        VertexBuffer_Vulkan() = default;
        virtual ~VertexBuffer_Vulkan() override;

        virtual void render(const RenderOptions* renderOptions, Material* material) override;

    protected:

        virtual bool initInternal(const VertexBufferData& data) override;
        virtual void onClearAsset() override;

    private:

        VulkanBuffer* m_VertexBuffer = nullptr;
        VulkanBuffer* m_IndexBuffer = nullptr;

        uint32 m_RenderElementsCount = 0;


        void clearVulkan();
    };
}

#endif

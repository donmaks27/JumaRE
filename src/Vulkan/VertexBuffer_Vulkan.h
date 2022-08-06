﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#if defined(JUMARE_ENABLE_VULKAN)

#include "../../include/JumaRE/vertex/VertexBuffer.h"

namespace JumaRenderEngine
{
    class VulkanBuffer;

    class VertexBuffer_Vulkan final : public VertexBuffer
    {
    public:
        VertexBuffer_Vulkan() = default;
        virtual ~VertexBuffer_Vulkan() override;

        virtual void render(const RenderOptions* renderOptions, Material* material) override;

    protected:

        virtual bool initInternal(VertexBufferData* verticesData) override;

    private:

        VulkanBuffer* m_VertexBuffer = nullptr;
        VulkanBuffer* m_IndexBuffer = nullptr;

        uint32 m_RenderElementsCount = 0;


        void clearVulkan();
    };
}

#endif
